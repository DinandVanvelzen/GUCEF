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

#ifndef GUCEF_CORE_CGENERICPLUGINMANAGER_H
#define GUCEF_CORE_CGENERICPLUGINMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Plugin manager for generic plugins
 *
 *  About Generic plugins:
 *  These are typically C++ modules that link back to the GUCEF modules and
 *  upon load integrate themselves in the framework. This allows a generic plugin
 *  to be/do just about anything but with the drawback that it has to link to the GUCEF
 *  modules and as such has a more limited lifespan than a C-interface plugin.
 */
class GUCEF_CORE_PUBLIC_CPP CGenericPluginManager : public CPluginManager
{
    public:

    virtual CString GetPluginType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TPluginPtr RegisterPlugin( void* modulePtr                   ,
                                       TPluginMetaDataPtr pluginMetaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnregisterPlugin( TPluginPtr plugin ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CGenericPluginManager( void );

    virtual ~CGenericPluginManager() GUCEF_VIRTUAL_OVERRIDE;

    private:
    
    CGenericPluginManager( const CGenericPluginManager& src );

    CGenericPluginManager& operator=( const CGenericPluginManager& src );
};


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CGENERICPLUGINMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 27-11-2004 :
        - Dinand: Initial implementation

-----------------------------------------------------------------------------*/
