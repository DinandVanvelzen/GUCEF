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

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#define GUCEF_CORE_CICODECINSTANCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_C_CODEC_PLUGIN_API_H
#include "gucefCORE_c_codec_plugin_api.h"   /* TCodecStatus, SCodecCallArgs, GUCEF_CODEC_PROCESS_* */
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

class CDynamicBuffer;

/*-------------------------------------------------------------------------*/

/**
 *  A prepared codec: its parameters (a key, a level) taken in once, ready for many calls.
 *  For hot paths and streams: a call does no lookups, and allocates nothing once the output buffer
 *  the caller reuses has grown to its working size.
 *
 *  One instance is used by one thread at a time. Instances are independent of each other.
 *  An instance keeps its codec, and so the plugin it may come from, alive.
 */
class GUCEF_CORE_PUBLIC_CPP CICodecInstance
{
    public:

    CICodecInstance( void );

    virtual ~CICodecInstance();

    /**
     *  One step of the codec
     *
     *  @param flags GUCEF_CODEC_PROCESS_*: BEGIN starts a message (an unfinished one is dropped), FINAL ends it and
     *         produces what comes at the end (a digest, a tag), UPDATE marks a call in between; plus one of ENCODE and
     *         DECODE. The input is always taken in. GUCEF_CODEC_PROCESS_ENCODE_MESSAGE / _DECODE_MESSAGE do it all at once.
     *  @param args the nonce and associated data of this call, NULL when the codec needs neither
     *  @param output the output is appended after its current data
     *  @return whether it succeeded, GetLastStatus() tells more
     */
    virtual bool Process( UInt32 flags                ,
                          const void* input           ,
                          UInt32 inputSize            ,
                          const SCodecCallArgs* args  ,
                          CDynamicBuffer& output      ) = 0;

    /** How many bytes encoding adds to a message (the tag of an AEAD cipher), 0 when none or when it is not fixed */
    virtual UInt32 GetOverhead( void ) const = 0;

    /** The outcome of the last Process() call */
    virtual TCodecStatus GetLastStatus( void ) const = 0;

    /** What went wrong in the last Process() call, empty when nothing did. Makes a string: for failures, not per call */
    virtual CString GetLastErrorMessage( void ) const = 0;

    private:

    CICodecInstance( const CICodecInstance& src );              /**< not implemented, instances hold state that does not copy */
    CICodecInstance& operator=( const CICodecInstance& src );   /**< not implemented, instances hold state that does not copy */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CICodecInstance, MT::CMutex > CICodecInstancePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */
