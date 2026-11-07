/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "CIPlugin.h"

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

CIPlugin::CIPlugin( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPlugin::CIPlugin( const CIPlugin& src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPlugin::~CIPlugin()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIPlugin&
CIPlugin::operator=( const CIPlugin& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CIPlugin::IsModuleInUse( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CIPlugin::OnBeforeModuleUnload( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/