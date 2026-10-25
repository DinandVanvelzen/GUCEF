/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H
#include "gucefCORE_CFaultTolerantConfigScope.h"
#define GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H
#endif /* GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H ? */

#ifndef GUCEF_CORE_CCONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CCONFIGSTORE_H
#endif /* GUCEF_CORE_CCONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

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

CFaultTolerantConfigScope::CFaultTolerantConfigScope( bool faultTolerant )
    : m_configStore( &CCoreGlobal::Instance()->GetConfigStore() )
{GUCEF_TRACE;

    m_configStore->BeginConfigurableScope( faultTolerant );
}

/*-------------------------------------------------------------------------*/

CFaultTolerantConfigScope::CFaultTolerantConfigScope( CConfigStore& configStore ,
                                                      bool faultTolerant        )
    : m_configStore( &configStore )
{GUCEF_TRACE;

    m_configStore->BeginConfigurableScope( faultTolerant );
}

/*-------------------------------------------------------------------------*/

CFaultTolerantConfigScope::~CFaultTolerantConfigScope()
{GUCEF_TRACE;

    m_configStore->EndConfigurableScope();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
