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

#ifndef GUCEF_CORE_CSTDCODECPLUGINITEM_H
#define GUCEF_CORE_CSTDCODECPLUGINITEM_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#ifndef GUCEF_CORE_CODECPLUGINLINK_H
#include "CodecPluginLink.h"
#define GUCEF_CORE_CODECPLUGINLINK_H
#endif /* GUCEF_CORE_CODECPLUGINLINK_H ? */

#ifndef GUCEF_CORE_CICODEC_H
#include "CICodec.h"
#define GUCEF_CORE_CICODEC_H
#endif /* GUCEF_CORE_CICODEC_H ? */

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

class CIOAccess;
class CStdCodecPlugin;

/*-------------------------------------------------------------------------*/

/**
 *  A codec of a plugin: redirects the codec interface to the functions of its TCodecPluginLink.
 *  Parameters reach the plugin as a variant map, its status and error text come back as the last status.
 */
class GUCEF_CORE_PUBLIC_CPP CStdCodecPluginItem : public CICodec
{
    public:

    virtual bool Encode( CIOAccess& source ,
                         CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Decode( CIOAccess& source ,
                         CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Encode( CIOAccess& source          ,
                         CIOAccess& dest            ,
                         const CValueList& params   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Decode( CIOAccess& source          ,
                         CIOAccess& dest            ,
                         const CValueList& params   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE;

    /** An instance of the plugin when it offers them, else the default instance. Keeps the plugin loaded */
    virtual bool CreateInstance( const CValueList& params       ,
                                 CICodecInstancePtr& instance   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The link of the codec as the host read it: what the plugin's struct did not cover is zero */
    const TCodecPluginLink& GetCodecPluginLink( void ) const;

    void* GetPluginData( void ) const;

    protected:
    friend class CStdCodecPlugin;

    CStdCodecPluginItem( CStdCodecPlugin* plugin          ,
                         void* pluginData                 ,
                         const TCodecPluginLink& link     );

    CStdCodecPluginItem( const CStdCodecPluginItem& src );

    virtual ~CStdCodecPluginItem() GUCEF_VIRTUAL_OVERRIDE;

    private:

    CStdCodecPluginItem& operator=( const CStdCodecPluginItem& src );   /**< not implemented */

    bool Run( bool encode, CIOAccess& source, CIOAccess& dest, const CValueList* params );

    private:

    TCodecPluginLink m_codecLink;
    void* m_pluginData;
    CStdCodecPlugin* m_plugin;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSTDCODECPLUGINITEM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 20-07-2005 :
        - Dinand: Added this class

-----------------------------------------------------------------------------*/
