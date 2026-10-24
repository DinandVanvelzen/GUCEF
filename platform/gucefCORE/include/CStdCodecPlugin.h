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

#ifndef GUCEF_CORE_CSTDCODECPLUGIN_H
#define GUCEF_CORE_CSTDCODECPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CICODECPLUGIN_H
#include "CICodecPlugin.h"
#define GUCEF_CORE_CICODECPLUGIN_H
#endif /* GUCEF_CORE_CICODECPLUGIN_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_C_CODEC_PLUGIN_API_H
#include "gucefCORE_c_codec_plugin_api.h"
#define GUCEF_CORE_C_CODEC_PLUGIN_API_H
#endif /* GUCEF_CORE_C_CODEC_PLUGIN_API_H ? */

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
 *  GUCEF CORE module plugin implementation for codecs: a module with the codec plugin C interface
 *  (gucefCORE_c_codec_plugin_api.h). Linking checks the interface version, initializes the plugin with the
 *  parameters of its metadata and registers its codecs in the codec registry, each with its priority.
 */
class GUCEF_CORE_PUBLIC_CPP CStdCodecPlugin : public CICodecPlugin
{
    public:

    CStdCodecPlugin( void );

    virtual ~CStdCodecPlugin() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetCodecs( CCodecSet& codecSet ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetCodecList( CCodecList& codecList ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Encode( CIOAccess& source         ,
                         CIOAccess& dest           ,
                         const CString& familyName ,
                         const CString& typeName   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Decode( CIOAccess& source         ,
                         CIOAccess& dest           ,
                         const CString& familyName ,
                         const CString& typeName   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TPluginMetaDataPtr GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetModulePointer( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Looks up the exports of the module and links it, see LinkApi() */
    virtual bool Link( void* modulePtr                   ,
                       TPluginMetaDataPtr pluginMetaData );

    virtual bool Unlink( void );

    virtual const CString& GetClassTypeName( void ) const;

    /** The interface version the linked plugin was built for (GUCEF_CODECPLUGIN_API_VERSION), 0 when none is linked */
    UInt32 GetApiVersion( void ) const;

    /** The shared pointer the plugin keeps of one of its codecs, NULL when the codec is not one of them */
    CCodecPtr GetCodecPtr( const CICodec* codec ) const;

    /** The metadata parameter that sets the priority of the plugin's codecs in the codec registry */
    static const CString CodecPriorityParamName;

    protected:

    CStdCodecPlugin( const CStdCodecPlugin& src );            /**< not implemented */
    CStdCodecPlugin& operator=( const CStdCodecPlugin& src ); /**< not implemented */

    /**
     *  Links the plugin through functions that were looked up already. The module pointer only identifies the module.
     *  Refuses a plugin of another major interface version without calling anything else of it,
     *  then calls Init with the parameters of the metadata and registers the codecs.
     */
    bool LinkApi( void* modulePtr                          ,
                  const TGucefCoreCodecPluginApi& api      ,
                  TPluginMetaDataPtr pluginMetaData        );

    CString GetDescription( void ) const;

    CString GetCopyright( void ) const;

    TVersion GetVersion( void ) const;

    bool IsLoaded( void ) const;

    void LinkCodecSet( void );

    void UnlinkCodecSet( void );

    bool Unlink( bool forceEvenIfInUse );

    /** The priority for a codec of this plugin: the metadata parameter, else the hint of its link, else the plugin default */
    Int32 GetCodecPriority( const TCodecPluginLink& codecLink ) const;

    protected:

    struct SPluginParams;

    CCodecSet m_codecSet;
    CCodecList m_codecList;
    TGucefCoreCodecPluginApi m_api;     /**< the functions of the linked plugin */
    void* m_soHandle;                   /**< access to the so module */
    void* m_pluginData;
    TPluginMetaDataStoragePtr m_metaData;
    TGucefCoreCApi m_coreCApi;          /**< handed to the plugin at Init, valid until Shutdown */
    SPluginParams* m_pluginParams;      /**< the metadata parameters as handed to the plugin at Init, valid until Shutdown */
    UInt32 m_apiVersion;
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CStdCodecPlugin, MT::CMutex >  CStdCodecPluginPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace CORE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSTDCODECPLUGIN_H ? */
