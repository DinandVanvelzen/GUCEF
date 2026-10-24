/*
 *  gucefIMAGE: GUCEF module providing image utilities
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_GENERICPLUGINAPI_H
#include "gucefCORE_GenericPluginAPI.h"
#define GUCEF_CORE_GENERICPLUGINAPI_H
#endif /* GUCEF_CORE_GENERICPLUGINAPI_H ? */

#ifndef GUCEF_CORE_CVALUEMAPBUILDER_H
#include "gucefCORE_CValueMapBuilder.h"
#define GUCEF_CORE_CVALUEMAPBUILDER_H
#endif /* GUCEF_CORE_CVALUEMAPBUILDER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_IMAGEDATA_H
#include "gucefIMAGE_c_imagedata.h"               /* C-style shared header for image related types */
#define GUCEF_IMAGE_IMAGEDATA_H
#endif /* GUCEF_IMAGE_IMAGEDATA_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#include "gucefIMAGE_CImageCodecPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CImageCodecPlugin::ImageCodecFamilyName = "ImageCodec";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EImageCodecPluginFuncPtrType
{
    ICPLUGINFUNCPTR_ENCODEIMAGE = 0     ,
    ICPLUGINFUNCPTR_DECODEIMAGE         ,
    ICPLUGINFUNCPTR_DECODEIMAGEMETADATA ,
    ICPLUGINFUNCPTR_FREEIMAGESTORAGE    ,

    ICPLUGINFUNCPTR_COUNT
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TIMGCODECPLUGFPTR_EncodeImage ) ( void* pluginData, void* codecData, const char* codecType, TImage* imageInput, CORE::TIOAccess* output ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TIMGCODECPLUGFPTR_DecodeImage ) ( void* pluginData, void* codecData, const char* codecType, CORE::TIOAccess* input, TImage** imageOutput, void** imageData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TIMGCODECPLUGFPTR_DecodeImageMetaData ) ( void* pluginData, void* codecData, const char* codecType, CORE::TIOAccess* input, CORE::TValueMapParserCallbacks* mapCallbacks ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TIMGCODECPLUGFPTR_FreeImageStorage ) ( TImage* image, void* imageData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageCodecPlugin::CImageCodecPlugin( void )
    : CORE::CStdCodecPlugin()
{GUCEF_TRACE;

    memset( m_icFuncPointers, 0, sizeof( m_icFuncPointers ) );
}

/*-------------------------------------------------------------------------*/

CImageCodecPlugin::~CImageCodecPlugin()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CImageCodecPlugin::Link( void* modulePtr                        ,
                        CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    // Unloading->Loading must be done explicitly
    if ( IsLoaded() ) return false;

    // First try and load the basic codec interface
    if ( !CORE::CStdCodecPlugin::Link( modulePtr, pluginMetaData ) ) return false;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecPlugin: Linking API using module pointer: " + CORE::PointerToString( modulePtr ) );

    // We have now successfully loaded the module itself
    // We will now try to lookup the function pointers
    m_icFuncPointers[ ICPLUGINFUNCPTR_ENCODEIMAGE ] = CORE::GetFunctionAddress( modulePtr                    ,
                                                                                "IMGCODECPLUGIN_EncodeImage" ,
                                                                                5*sizeof(void*)              ).funcPtr;
    m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGE ] = CORE::GetFunctionAddress( modulePtr                    ,
                                                                                "IMGCODECPLUGIN_DecodeImage" ,
                                                                                6*sizeof(void*)              ).funcPtr;
    m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGEMETADATA ] = CORE::GetFunctionAddress( modulePtr                            ,
                                                                                        "IMGCODECPLUGIN_DecodeImageMetaData" ,
                                                                                        6*sizeof(void*)                      ).funcPtr;
    m_icFuncPointers[ ICPLUGINFUNCPTR_FREEIMAGESTORAGE ] = CORE::GetFunctionAddress( modulePtr           ,
                                                                                     "IMGCODECPLUGIN_FreeImageStorage" ,
                                                                                     2*sizeof( void* )                 ).funcPtr;

    // Verify that all function pointers are loaded correctly
    if ( ( GUCEF_NULL != m_icFuncPointers[ ICPLUGINFUNCPTR_ENCODEIMAGE ] )         &&
         ( GUCEF_NULL != m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGE ] )         &&
         ( GUCEF_NULL != m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGEMETADATA ] ) &&
         ( GUCEF_NULL != m_icFuncPointers[ ICPLUGINFUNCPTR_FREEIMAGESTORAGE ] )     )
    {
            // We have loaded & linked our plugin module
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecPlugin: Successfully linked module : " + CORE::PointerToString( modulePtr ) );
            return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecPlugin: Failed to locate the required exports in module: " + CORE::PointerToString( modulePtr ) );
    }

    // We failed to link the functions :(
    memset( m_icFuncPointers, 0, sizeof( m_icFuncPointers ) );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImageCodecPlugin::Unlink( void )
{GUCEF_TRACE;

    if ( !CORE::CStdCodecPlugin::Unlink() ) return false;
    memset( m_icFuncPointers, 0, sizeof( m_icFuncPointers ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImageCodecPlugin::Encode( const CImage& inputImage       ,
                           CORE::CIOAccess& encodedOutput ,
                           const CString& typeName        )
{GUCEF_TRACE;

    // a plugin that is not linked (or lacks the function) has nothing to call
    if ( GUCEF_NULL == m_icFuncPointers[ ICPLUGINFUNCPTR_ENCODEIMAGE ] )
        return false;

    TImage* cStyleImage = inputImage.CreateCStyleAccess();

    bool success = ( 0 != ( (TIMGCODECPLUGFPTR_EncodeImage) m_icFuncPointers[ ICPLUGINFUNCPTR_ENCODEIMAGE ] )( m_pluginData                 ,
                                                                                                               NULL                         ,
                                                                                                               typeName.C_String()          ,
                                                                                                               cStyleImage                  ,
                                                                                                               encodedOutput.CStyleAccess() ) );
    inputImage.FreeCStyleAccess( cStyleImage );

    return success;
}

/*-------------------------------------------------------------------------*/

bool
CImageCodecPlugin::Decode( CORE::CIOAccess& encodedInput ,
                           CImage& outputImage           ,
                           const CString& typeName       )
{GUCEF_TRACE;

    // a plugin that is not linked (or lacks the function) has nothing to call
    if ( GUCEF_NULL == m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGE ] )
        return false;

     TImage* cStyleImage = NULL;
     void* cStyleImageData = NULL;

     if ( 0 != ( (TIMGCODECPLUGFPTR_DecodeImage) m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGE ] )( m_pluginData                 ,
                                                                                                    NULL                         ,
                                                                                                    typeName.C_String()          ,
                                                                                                    encodedInput.CStyleAccess()  ,
                                                                                                    &cStyleImage                 ,
                                                                                                    &cStyleImageData             ) )
    {
        if ( NULL != cStyleImage )
        {
            outputImage.Assign( *cStyleImage );
            ( (TIMGCODECPLUGFPTR_FreeImageStorage) m_icFuncPointers[ ICPLUGINFUNCPTR_FREEIMAGESTORAGE ] )( cStyleImage, cStyleImageData );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImageCodecPlugin::Encode( CORE::CIOAccess& source ,
                           CORE::CIOAccess& dest   ,
                           const CString& typeName )
{GUCEF_TRACE;

    return CORE::CStdCodecPlugin::Encode( source               ,
                                          dest                 ,
                                          ImageCodecFamilyName ,
                                          typeName             );
}

/*-------------------------------------------------------------------------*/

bool
CImageCodecPlugin::Decode( CORE::CIOAccess& source ,
                           CORE::CIOAccess& dest   ,
                           const CString& typeName )
{GUCEF_TRACE;

    return CORE::CStdCodecPlugin::Decode( source               ,
                                          dest                 ,
                                          ImageCodecFamilyName ,
                                          typeName             );
}

/*-------------------------------------------------------------------------*/

bool 
CImageCodecPlugin::DecodeMetaData( CORE::CIOAccess& encodedInput ,
                                   CORE::CValueList& metaData    ,
                                   const CString& typeName       )
{GUCEF_TRACE;

    // a plugin that is not linked (or lacks the function) has nothing to call
    if ( GUCEF_NULL == m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGEMETADATA ] )
        return false;

    CORE::CValueMapBuilder mapBuilder;

    if ( 0 != ( (TIMGCODECPLUGFPTR_DecodeImageMetaData) m_icFuncPointers[ ICPLUGINFUNCPTR_DECODEIMAGEMETADATA ] )( m_pluginData               ,
                                                                                                                GUCEF_NULL                    ,
                                                                                                                typeName.C_String()           ,
                                                                                                                encodedInput.CStyleAccess()   ,
                                                                                                                &mapBuilder.GetCStyleAccess() ) )
    {
        metaData = mapBuilder.map;
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
