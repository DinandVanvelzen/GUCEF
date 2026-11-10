/*
 *  gucefINPUT: GUCEF module providing input device interaction
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

#ifndef GUCEF_INPUT_CINPUTDRIVERPLUGIN_H
#define GUCEF_INPUT_CINPUTDRIVERPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef DVOSWRAP_H
#include "DVOSWRAP.h"            /* OS wrapping functionality */
#define DVOSWRAP_H
#endif /* DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_INPUT_CINPUTDRIVER_H
#include "gucefINPUT_CInputDriver.h"
#define GUCEF_INPUT_CINPUTDRIVER_H
#endif /* GUCEF_INPUT_CINPUTDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Class that connects the input driver interface to a plugin mechanism so
 *      that C-style plugins can be used as an input driver.
 */
class GUCEF_INPUT_PUBLIC_CPP CInputDriverPlugin : public CInputDriver   ,
                                                  public CORE::CIPlugin
{
    public:

    CInputDriverPlugin( void );

    virtual ~CInputDriverPlugin();

    bool Link( void* modulePtr                         ,
               CORE::TPluginMetaDataPtr pluginMetaData );

    void Unlink( void );

    virtual CORE::TPluginMetaDataPtr GetMetaData( void ) const;

    virtual void* GetModulePointer( void );

    virtual const CORE::TVersion* GetVersion( void );

    virtual CString GetName( void ) const;

    protected:

    virtual bool OnUpdate( const UInt64 tickcount               ,
                           const Float64 updateDeltaInMilliSecs ,
                           CInputContext* context               );

    virtual CInputContext* CreateContext( const CORE::CValueList& params );

    virtual void DeleteContext( CInputContext* context );

    private:

    static void CreateArgMatrix( const CORE::CValueList& params, char**& argv, int& argc );
    static void DestroyArgMatrix( char** argv, int argc );

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnMouseButtonDown( void* userData           ,
                       const Int32 deviceID     ,
                       const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnMouseButtonUp( void* userData           ,
                     const Int32 deviceID     ,
                     const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnMouseMove( void* userData        ,
                 const Int32 deviceID  ,
                 const Int32 xPos      ,
                 const Int32 yPos      ,
                 const Int32 xDelta    ,
                 const Int32 yDelta    ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnMouseVarChanged( void* userData         ,
                       const Int32 deviceID   ,
                       const UInt32 varIndex  ,
                       const Int32 value      ,
                       const Int32 valueDelta ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnKeyboardText( void* userData        ,
                    const Int32 deviceID  ,
                    const char* utf8Text  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnKeyboardKeyDown( void* userData            ,
                       const Int32 deviceID      ,
                       const KeyCode keyCode     ,
                       const UInt32 keyModStates ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnKeyboardKeyUp( void* userData            ,
                     const Int32 deviceID      ,
                     const KeyCode keyCode     ,
                     const UInt32 keyModStates ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnDeviceBooleanOff( void* userData          ,
                        const Int32 deviceID    ,
                        const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnDeviceBooleanOn( void* userData          ,
                       const Int32 deviceID    ,
                       const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnDeviceVarChanged( void* userData          ,
                        const Int32 deviceID    ,
                        const UInt32 stateindex ,
                        const Float32 value     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnMouseAttached( void* userData       ,
                     const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnMouseDetached( void* userData       ,
                     const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnKeyboardAttached( void* userData       ,
                        const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnKeyboardDetached( void* userData       ,
                        const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnDeviceAttached( void* userData       ,
                        const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    static void GUCEF_PLUGIN_CALLSPEC_PREFIX
    OnDeviceDetached( void* userData        ,
                      const Int32 deviceID  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

    CInputDriverPlugin( const CInputDriverPlugin& src );            /**< not applicable, do not use */
    CInputDriverPlugin& operator=( const CInputDriverPlugin& src ); /**< not applicable, do not use */

    private:

    void* m_sohandle;
    void* m_plugdata;
    CORE::TDefaultFuncPtr m_fptable[ 8 ];
    CORE::TPluginMetaDataStoragePtr m_metaData;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CInputDriverPlugin, MT::CMutex > TInputDriverPluginPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_INPUT_CINPUTDRIVERPLUGIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-09-2005 :
        - Initial implementation

-----------------------------------------------------------------------------*/
