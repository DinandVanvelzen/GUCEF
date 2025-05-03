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

#include "gucefCORE_CGlobalDataDrivenDStoreCodecFactory.h"

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

CGlobalDataDrivenDStoreCodecFactory::CGlobalDataDrivenDStoreCodecFactory( void )
    : CDataDrivenDStoreCodecFactory()
    , CGloballyConfigurable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGlobalDataDrivenDStoreCodecFactory::~CGlobalDataDrivenDStoreCodecFactory( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString& 
CGlobalDataDrivenDStoreCodecFactory::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return CDataDrivenDStoreCodecFactory::GetClassTypeName();
}

/*-------------------------------------------------------------------------*/

bool
CGlobalDataDrivenDStoreCodecFactory::SaveConfig( CDataNode& cfg ) const
{GUCEF_TRACE;

    return false; // not implemented yet
}

/*-------------------------------------------------------------------------*/

bool
CGlobalDataDrivenDStoreCodecFactory::LoadConfig( const CDataNode& cfg )
{GUCEF_TRACE;

    const CDataNode* globalConfig = cfg.Find( "GlobalDataDrivenDStoreCodecFactory" );
    if ( GUCEF_NULL != globalConfig )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "GlobalDataDrivenDStoreCodecFactory:LoadConfig: Loading config" );
        
        CDataDrivenDStoreCodecFactory::LoadConfig( cfg );
        
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "GlobalDataDrivenDStoreCodecFactory:LoadConfig: Finished loading config" );
    }
    return true; // No config provided, treat as no-op since its optional
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
