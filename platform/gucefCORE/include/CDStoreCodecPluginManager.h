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

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"   /* often used gucefCORE macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"    /* base class for plugin managers */
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CDSTORECODECPLUGIN_H
#include "CDStoreCodecPlugin.h"
#define GUCEF_CORE_CDSTORECODECPLUGIN_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGIN_H */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"             /* abstract base class interface for configurable objects */
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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
 *  Internal implementation class for managing DStoreCodec plugins
 *  These codec plugins use their own specific C interface which is intended to remain stable for a long time
 *  Thus the plugin manager is responsible for loading and unloading the plugins and providing them to the outside world,
 *  as the party aware of the specifics of these C API plugins.
 */
class GUCEF_HIDDEN CDStoreCodecPluginManager : public CPluginManager
{
    public:

    typedef CTSharedPtr< CDStoreCodecPlugin, MT::CMutex > TDStoreCodecPluginPtr;

    TDStoreCodecPluginPtr GetCodec( const CString& codectype ) const;

    virtual CString GetPluginType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TPluginPtr RegisterPlugin( void* modulePtr                   ,
                                       TPluginMetaDataPtr pluginMetaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool UnregisterPlugin( TPluginPtr plugin ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CDStoreCodecPluginManager( void );

    virtual ~CDStoreCodecPluginManager() GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef std::set< TDStoreCodecPluginPtr > TDStoreCodecPluginSet;

    bool IsPluginLoaded( const CString& path );

    CDStoreCodecPluginManager( const CDStoreCodecPluginManager& src );
    
    CDStoreCodecPluginManager& operator=( const CDStoreCodecPluginManager& src );

    TDStoreCodecPluginSet _codecs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 21-09-2005 :
        - Added check that makes sure the same plugin module is not loaded twice
- 04-04-2005 :
        - Initial version.

-----------------------------------------------------------------------------*/
