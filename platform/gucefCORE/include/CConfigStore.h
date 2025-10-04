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

#ifndef GUCEF_CORE_CONFIGSTORE_H
#define GUCEF_CORE_CONFIGSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/*
 *      Forward declaration of the abstract base for configurable items.
 */
class CIConfigurable;
class CGloballyConfigurable;
class CDataNode;

/*-------------------------------------------------------------------------*/

/**
 *  Centralized configuration control.
 *  Allows you load and save the config of all configureable items that
 *  have global config usage enabled.
 * 
 *  Note that you can specify multiple config files which will all get overlayed into a singlular
 *  document and loaded as a consolidated global config.
 *  Distinct from that is the concept of the bootstrap config which is explicitly a distinct load and application
 *  of config before the main config is loaded. Main use-case is loading plugins and the like which are in turn
 *  needed to load and or apply the global config
 */
class GUCEF_CORE_PUBLIC_CPP CConfigStore : public CTSGNotifier
{
    public:

    static const CEvent GlobalBootstrapConfigLoadStartingEvent;
    static const CEvent GlobalBootstrapConfigLoadCompletedEvent;
    static const CEvent GlobalBootstrapConfigLoadFailedEvent;

    static const CEvent GlobalConfigLoadStartingEvent;
    static const CEvent GlobalConfigLoadCompletedEvent;
    static const CEvent GlobalConfigLoadFailedEvent;

    static void RegisterEvents( void );

    void SetBootstrapConfigFile( const CString& filepath );

    CString GetBootstrapConfigFile( void ) const;

    void SetConfigFile( const CString& filepath ,
                        bool isAddative = false );

    CString GetConfigFile( void ) const;

    void SetConfigFiles( const CString::StringVector& filepaths ,
                         bool isAddative = false                );

    CString::StringVector GetConfigFiles( void ) const;

    bool SaveConsolidatedConfig( const CString& name  ,
                                 bool preserve = true );

    /**
     *  Persists the given config as the new consolidated config
     *  Does NOT apply the config. Caller can do that independently using ApplyConsolidatedConfig()
     */ 
    bool SaveConsolidatedConfig( const CDataNode& newCfg );

    /**
     *  Applies the given config to all non-bootstrap global configurables
     *  Does NOT persist the consolidated config. Caller can do that independently using SaveConsolidatedConfig()
     */ 
    bool ApplyConsolidatedConfig( const CDataNode& newCfg );

    bool LoadConfig( CDataNode* loadedConfig = GUCEF_NULL );

    CString GetConsolidatedConfigFile( void ) const;

    CString GetConsolidatedConfigFileCodecType( void ) const;

    void SetCodec( const CString& codectype );

    CString GetCodec( void ) const;

    bool IsGlobalBootstrapConfigLoadInProgress( void ) const;
    
    bool IsGlobalConfigLoadInProgress( void ) const;

    bool IsGlobalConfigLoaded( void ) const;
    
    bool IsGlobalBootstrapConfigIsLoaded( void ) const;

    private:

    friend class CGloballyConfigurable;

    void Register( CGloballyConfigurable* configobj );

    bool Unregister( CGloballyConfigurable* configobj );

    private:
    friend class CCoreGlobal;

    CConfigStore( void );

    virtual ~CConfigStore();

    private:

    bool LoadConfigData( const CString& cfgFilepath , 
                         CDataNode& loadedConfig    );

    bool ApplyConfigData( const CDataNode& loadedConfig ,
                          bool isBootstrap              ,
                          bool hasBootstrap             );

    CConfigStore( const CConfigStore& src );             /**< not implemented */
    CConfigStore& operator=( const CConfigStore& src );  /**< not implemented */

    private:

    typedef GUCEF::set< CGloballyConfigurable* > TConfigurableSet;

    CString _codectype;
    CString m_bootstrapConfigFile;
    CString m_consolidatedConfigFile;
    CString::StringVector m_configfiles;
    TConfigurableSet m_configureables;
    TConfigurableSet m_newConfigureables;
    bool m_isBusyLoadingConfig;
    bool m_globalConfigIsLoaded;
    bool m_globalBootstrapConfigIsLoaded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-02-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/
