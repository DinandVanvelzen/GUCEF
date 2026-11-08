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

#ifndef GUCEF_CORE_CCONFIGSTORE_H
#include "CConfigStore.h"       /* central configuration control */
#define GUCEF_CORE_CCONFIGSTORE_H
#endif /* GUCEF_CORE_CCONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefCORE_CGloballyConfigurable.h" 

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGloballyConfigurable::CGloballyConfigurable( bool includeInBootstrapPhase )
    : CIConfigurable()
    , m_includeInBootstrapPhase( includeInBootstrapPhase )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetConfigStore().Register( this );
}

/*-------------------------------------------------------------------------*/

CGloballyConfigurable::CGloballyConfigurable( const CGloballyConfigurable& src )
    : CIConfigurable( src )
    , m_includeInBootstrapPhase( src.m_includeInBootstrapPhase )
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetConfigStore().Register( this );
}

/*-------------------------------------------------------------------------*/

CGloballyConfigurable::~CGloballyConfigurable()
{GUCEF_TRACE;

    CORE::CCoreGlobal::Instance()->GetConfigStore().Unregister( this );
}

/*-------------------------------------------------------------------------*/

CGloballyConfigurable&
CGloballyConfigurable::operator=( const CGloballyConfigurable& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CIConfigurable::operator=( src );
    }

    return *this;
}

/*-------------------------------------------------------------------------*/

void
CGloballyConfigurable::UnregisterFromConfigStore( void )
{GUCEF_TRACE;

    // a second time, from the destructor of this class, finds nothing to do
    CORE::CCoreGlobal::Instance()->GetConfigStore().Unregister( this );
}

/*-------------------------------------------------------------------------*/

bool
CGloballyConfigurable::IsGlobalBootstrapConfigLoadInProgress( void )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetConfigStore().IsGlobalBootstrapConfigLoadInProgress();
}

/*-------------------------------------------------------------------------*/

bool
CGloballyConfigurable::IsGlobalConfigLoadInProgress( void )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetConfigStore().IsGlobalConfigLoadInProgress();
}

/*-------------------------------------------------------------------------*/

bool 
CGloballyConfigurable::IsIncludedInGlobalBootstrapConfigLoad( void ) const
{GUCEF_TRACE;

    return m_includeInBootstrapPhase;
}

/*-------------------------------------------------------------------------*/

bool
CGloballyConfigurable::SaveConfig( CDataNode& cfg ) const
{GUCEF_TRACE;

    // default is that we wish to save nothing which is always successful
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGloballyConfigurable::LoadConfig( const CDataNode& cfg )
{GUCEF_TRACE;

    // default is that we have no mandatory/required settings and thus are always successful
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
