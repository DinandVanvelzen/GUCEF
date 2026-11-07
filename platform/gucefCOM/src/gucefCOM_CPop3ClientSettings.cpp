/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCOM_CPop3ClientSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPop3ClientSettings::CPop3ClientSettings( void )
    : CEMailClientSettings()
    , useApop( false )
    , useTop( true )
    , maxMessagesPerSession( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3ClientSettings::CPop3ClientSettings( const CPop3ClientSettings& src )
    : CEMailClientSettings( src )
    , useApop( src.useApop )
    , useTop( src.useTop )
    , maxMessagesPerSession( src.maxMessagesPerSession )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3ClientSettings::~CPop3ClientSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3ClientSettings&
CPop3ClientSettings::operator=( const CPop3ClientSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CEMailClientSettings::operator=( src );
        useApop = src.useApop;
        useTop = src.useTop;
        maxMessagesPerSession = src.maxMessagesPerSession;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt16
CPop3ClientSettings::GetDefaultPort( EMailTlsMode mode ) const
{GUCEF_TRACE;

    switch ( mode )
    {
        case MAILTLS_IMPLICIT: return 995;
        case MAILTLS_NONE: return 110;
        default: return 110;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPop3ClientSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !CEMailClientSettings::SaveConfig( config ) )
        return false;
    config.SetAttribute( "useApop", useApop );
    config.SetAttribute( "useTop", useTop );
    config.SetAttribute( "maxMessagesPerSession", maxMessagesPerSession );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPop3ClientSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !CEMailClientSettings::LoadConfig( config ) )
        return false;
    useApop = config.GetAttributeValueOrChildValueByName( "useApop" ).AsBool( useApop, true );
    useTop = config.GetAttributeValueOrChildValueByName( "useTop" ).AsBool( useTop, true );
    maxMessagesPerSession = config.GetAttributeValueOrChildValueByName( "maxMessagesPerSession" ).AsUInt32( maxMessagesPerSession, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPop3ClientSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CPop3ClientSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
