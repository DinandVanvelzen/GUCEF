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

#ifndef GUCEF_CORE_CGENERICPLUGIN_H
#define GUCEF_CORE_CGENERICPLUGIN_H

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

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

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

class CGenericPluginManager;

/*-------------------------------------------------------------------------*/

/**
 *  Class wrapping the generic plugin interface into an object
 *
 *  About Generic plugins:
 *  These are typicly C++ modules that link back to the GUCEF modules and
 *  uppon load integrate themselves in the framework. This allows a generic plugin
 *  to be/do just about anything but with the drawback that it has to link to the GUCEF
 *  modules and as such has a more limited lifespan as a binary plugin.
 *  Its compatiblity is very narrow and any change in the GUCEF and related libraries
 *  can break compatibility witha  generic plugin. As such only use generic plugins in cases
 *  were you are going to compile them anew for every new revision of the platform.
 */
class GUCEF_CORE_PUBLIC_CPP CGenericPlugin : public CIPlugin
{
    public:

    CGenericPlugin( void );

    virtual ~CGenericPlugin() GUCEF_VIRTUAL_OVERRIDE;

    virtual TPluginMetaDataPtr GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetModulePointer( void ) GUCEF_VIRTUAL_OVERRIDE;

    void SetPluginParams( const CValueList& pluginParams );

    const CValueList& GetPluginParams( void ) const;

    bool Link( void* modulePtr                   ,
               TPluginMetaDataPtr pluginMetaData );

    bool Unlink( void );

    private:

    CGenericPlugin( const CGenericPlugin& src );

    CGenericPlugin& operator=( const CGenericPlugin& src );

    bool IsLoaded( void ) const;

    CString GetDescription( void ) const;

    CString GetCopyright( void ) const;

    TVersion GetVersion( void ) const;

    private:

    void* m_moduleHandle;
    TDefaultFuncPtr m_funcPointers[ 6 ];
    CString m_modulePath;
    CValueList m_params;
    TPluginMetaDataStoragePtr m_metaData;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CGenericPlugin, MT::CMutex >  CGenericPluginPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CGENERICPLUGIN_H ? */

