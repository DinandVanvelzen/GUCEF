/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_C_CODEC_PLUGIN_API_H
#define GUCEF_CORE_C_CODEC_PLUGIN_API_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"                   /* TVersion */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_C_API_H
#include "gucefCORE_c_api.h"            /* TGucefCoreCApi, logging for plugins */
#define GUCEF_CORE_C_API_H
#endif /* GUCEF_CORE_C_API_H ? */

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"       /* TIOAccess */
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

#ifndef GUCEF_CORE_C_VARIANTMAP_H
#include "gucefCORE_c_variantmap.h"     /* TVariantMapApi */
#define GUCEF_CORE_C_VARIANTMAP_H
#endif /* GUCEF_CORE_C_VARIANTMAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The version of this interface, as CODECPLUGIN_GetApiVersion() returns it: ( major << 16 ) | minor.
 *  The host refuses a plugin of another major version. A minor version only adds at the end of structs.
 */
#define GUCEF_CODECPLUGIN_API_VERSION_MAJOR             2
#define GUCEF_CODECPLUGIN_API_VERSION_MINOR             0
#define GUCEF_CODECPLUGIN_API_VERSION                   ( ( GUCEF_CODECPLUGIN_API_VERSION_MAJOR << 16 ) | GUCEF_CODECPLUGIN_API_VERSION_MINOR )
#define GUCEF_CODECPLUGIN_API_VERSION_GET_MAJOR( v )    ( ( (UInt32) (v) ) >> 16 )
#define GUCEF_CODECPLUGIN_API_VERSION_GET_MINOR( v )    ( ( (UInt32) (v) ) & 0xFFFF )

/* What a codec can do, TCodecPluginLink::capabilities */
#define GUCEF_CODEC_CAPABILITY_ENCODE           0x01    /* encode is there */
#define GUCEF_CODEC_CAPABILITY_DECODE           0x02    /* decode is there */
#define GUCEF_CODEC_CAPABILITY_INSTANCES        0x04    /* createInstance, instanceProcess and destroyInstance are there: incremental and streaming use */
#define GUCEF_CODEC_CAPABILITY_DETERMINISTIC    0x08    /* the same input and parameters always give the same output */
#define GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY     0x10    /* an instance call must have both BEGIN and FINAL: the whole message in one call (AEAD) */

/*
 *  Flags of instanceProcess. The input is always taken in.
 *  BEGIN starts a message (an unfinished one is dropped), FINAL ends it and produces what comes at the end
 *  (a digest, a tag), UPDATE marks a call in between and has no effect of its own.
 *  Exactly one of ENCODE and DECODE is set.
 */
#define GUCEF_CODEC_PROCESS_BEGIN               0x01
#define GUCEF_CODEC_PROCESS_UPDATE              0x02
#define GUCEF_CODEC_PROCESS_FINAL               0x04
#define GUCEF_CODEC_PROCESS_ENCODE              0x10
#define GUCEF_CODEC_PROCESS_DECODE              0x20
#define GUCEF_CODEC_PROCESS_ENCODE_MESSAGE      ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL | GUCEF_CODEC_PROCESS_ENCODE )
#define GUCEF_CODEC_PROCESS_DECODE_MESSAGE      ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL | GUCEF_CODEC_PROCESS_DECODE )

/* The size of the message buffer in TCodecErrorInfo */
#define GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE      256

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The outcome of a call. Values are never renumbered, new ones are only added.
 */
enum ECodecStatus
{
    GUCEF_CODEC_STATUS_OK                       = 0,    /**< done */
    GUCEF_CODEC_STATUS_FAILED                   = 1,    /**< failed for another reason: I/O, out of memory, the OS refused */
    GUCEF_CODEC_STATUS_INVALID_INPUT            = 2,    /**< the input is not what the codec decodes: corrupt, truncated */
    GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED    = 3,    /**< an AEAD tag or a signature does not match */
    GUCEF_CODEC_STATUS_MISSING_PARAMETER        = 4,    /**< a required parameter is absent */
    GUCEF_CODEC_STATUS_INVALID_PARAMETER        = 5,    /**< a parameter has a wrong size, type or value */
    GUCEF_CODEC_STATUS_UNSUPPORTED              = 6,    /**< the codec does not do that: decoding a hash, a capability it lacks, an index past the end */
    GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL         = 7     /**< the output did not fit, see instanceProcess */
};
typedef enum ECodecStatus TCodecStatus;

/*-------------------------------------------------------------------------*/

/**
 *  Error details, owned by the caller. A plugin writes a zero terminated message on failure when the pointer is not NULL.
 */
struct SCodecErrorInfo
{
    UInt32 structSize;                                      /**< sizeof( TCodecErrorInfo ) as the caller was built */
    char message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
};
typedef struct SCodecErrorInfo TCodecErrorInfo;

/*-------------------------------------------------------------------------*/

/**
 *  What changes per call of an instance: the nonce and the associated data (AAD) of an AEAD message.
 *  Plain pointers owned by the caller, valid for the call only. Members only ever get added at the end.
 */
struct SCodecCallArgs
{
    UInt32 structSize;          /**< sizeof( TCodecCallArgs ) as the caller was built */
    const void* nonce;
    UInt32 nonceSize;
    const void* aad;
    UInt32 aadSize;
};
typedef struct SCodecCallArgs TCodecCallArgs;

/*-------------------------------------------------------------------------*/

/**
 *  The description of one parameter of a codec. The strings are the plugin's own.
 */
struct SCodecParameterInfo
{
    UInt32 structSize;          /**< sizeof( TCodecParameterInfo ) as the host was built, the plugin fills what both know */
    const char* name;
    UInt8 typeId;               /**< GUCEF_DATATYPE_*: GUCEF_DATATYPE_BINARY_BLOB for keys and nonces */
    UInt8 required;             /**< 1 or 0 */
    const char* defaultValue;   /**< as text, NULL when there is none */
    const char* description;
};
typedef struct SCodecParameterInfo TCodecParameterInfo;

/*-------------------------------------------------------------------------*/

/*
 *  The functions behind a codec, referred to from its link. 'pluginData' is what Init gave, 'codecData' is the
 *  link's own. 'params' may be NULL and maps text keys to variants (see the rules below). 'error' may be NULL.
 *
 *  Encode / Decode: one shot from 'input' to 'output', reentrant.
 *  GetParameterInfo: OK for every index below the parameter count, UNSUPPORTED after. Reentrant.
 *  CreateInstance: a prepared codec with the parameters taken in (copied where kept). Reentrant.
 *  InstanceProcess: one step of an instance, buffer based: 'output' has room for 'outputCapacity' bytes,
 *      '*outputSize' gets what was written. 'args' may be NULL.
 *      The host gives at least the bound of GetOutputSizeBound for the call. A codec without a bound (decompression)
 *      that runs out of room returns OUTPUT_TOO_SMALL after taking in all of the input, with '*outputSize' what it
 *      wrote, and keeps the rest: the host calls again with more room, without BEGIN and without input (FINAL again
 *      when it was set), until the status is OK. A codec with GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY checks the room
 *      first and changes nothing when it is too small, so the host repeats the call with more room.
 *  DestroyInstance: frees an instance.
 *  GetOutputSizeBound: the most one call with 'inputSize' bytes of input can write. 'instanceData' is NULL for a one
 *      shot call of that input. UNSUPPORTED when the output cannot be bounded beforehand. Reentrant for one shot use.
 */
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Encode )             ( void* pluginData, void* codecData, const TVariantMapApi* params, TIOAccess* input, TIOAccess* output, TCodecErrorInfo* error ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Decode )             ( void* pluginData, void* codecData, const TVariantMapApi* params, TIOAccess* input, TIOAccess* output, TCodecErrorInfo* error ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetParameterInfo )   ( void* pluginData, void* codecData, UInt32 index, TCodecParameterInfo* info ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_CreateInstance )     ( void* pluginData, void* codecData, const TVariantMapApi* params, void** instanceData, TCodecErrorInfo* error ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_InstanceProcess )    ( void* instanceData, UInt32 flags, const void* input, UInt32 inputSize, const TCodecCallArgs* args, void* output, UInt32 outputCapacity, UInt32* outputSize, TCodecErrorInfo* error ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_DestroyInstance )            ( void* instanceData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetOutputSizeBound ) ( void* pluginData, void* codecData, void* instanceData, UInt32 flags, UInt32 inputSize, UInt32* bound ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *  One codec of a plugin. The plugin owns it (static data is fine) and keeps it valid until Shutdown.
 *  The host reads only what 'structSize' covers and treats the rest as absent, so fields only ever go at the end.
 *  Function pointers marked optional may be NULL; encode or decode may be NULL when its capability is not set.
 */
struct SCodecPluginLink
{
    UInt32 structSize;                                      /**< sizeof( TCodecPluginLink ) as the plugin was built */
    const char* codecFamily;
    const char* codecType;
    void* codecData;                                        /**< the plugin's own, identifies the codec in calls */
    UInt32 capabilities;                                    /**< GUCEF_CODEC_CAPABILITY_* */
    Int32 priorityHint;                                     /**< 0: the host default for plugins */
    UInt32 overhead;                                        /**< bytes encoding adds to a message (an AEAD tag), 0 when none or not fixed */
    TCODECPLUGFPTR_Encode encode;
    TCODECPLUGFPTR_Decode decode;
    TCODECPLUGFPTR_GetParameterInfo getParameterInfo;       /**< optional */
    TCODECPLUGFPTR_CreateInstance createInstance;           /**< optional, together with instanceProcess and destroyInstance */
    TCODECPLUGFPTR_InstanceProcess instanceProcess;
    TCODECPLUGFPTR_DestroyInstance destroyInstance;
    TCODECPLUGFPTR_GetOutputSizeBound getOutputSizeBound;   /**< optional */
};
typedef struct SCodecPluginLink TCodecPluginLink;

/*-------------------------------------------------------------------------*/

/*
 *  The functions a codec plugin exports. The host looks them up by name: the type name without the leading
 *  'TCODECPLUGFPTR' and with 'CODECPLUGIN' in front, so 'CODECPLUGIN_GetApiVersion' and so on. All are required.
 *
 *  Rules:
 *  - CODECPLUGIN_GetApiVersion returns GUCEF_CODECPLUGIN_API_VERSION. The host calls nothing else of a plugin of
 *    another major version, or of a plugin without this export.
 *  - Init gets the core C API (logging) and the parameters of the plugin's metadata as a map of text; both stay
 *    valid until Shutdown. 'pluginParams' may be NULL.
 *  - Strings and links the plugin hands out are its own and stay valid until Shutdown.
 *  - encode, decode, getParameterInfo, createInstance and getOutputSizeBound must be reentrant: the host calls them
 *    from several threads at once. One instance is used by one thread at a time.
 *  - Published structs start with their size and only grow at the end.
 *  - Status values are never renumbered.
 *  - Parameters are a map with text keys. Values are variants: text and numbers as GUCEF_DATATYPE_*, keys and nonces
 *    as GUCEF_DATATYPE_BINARY_BLOB or the bytes of a string. They stay valid for the call only, so an instance
 *    copies what it keeps. Parameters a codec does not know are ignored.
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetApiVersion )      ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Init )         ( void** plugdata, TGucefCoreCApi* coreApi, const TVariantMapApi* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Shutdown )     ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Description )   ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Copyright )     ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TVersion ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Version )          ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetCodecCount )      ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TCodecStatus ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetCodecLink ) ( void* plugdata, UInt32 index, const TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *  All the functions of a codec plugin, as the host found them.
 *  The counterpart of TGucefCoreCDStorePluginApi.
 */
struct SGucefCoreCodecPluginApi
{
    TCODECPLUGFPTR_GetApiVersion GetApiVersion;
    TCODECPLUGFPTR_Init Init;
    TCODECPLUGFPTR_Shutdown Shutdown;
    TCODECPLUGFPTR_Description Description;
    TCODECPLUGFPTR_Copyright Copyright;
    TCODECPLUGFPTR_Version Version;
    TCODECPLUGFPTR_GetCodecCount GetCodecCount;
    TCODECPLUGFPTR_GetCodecLink GetCodecLink;
};
typedef struct SGucefCoreCodecPluginApi TGucefCoreCodecPluginApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_CODEC_PLUGIN_API_H ? */
