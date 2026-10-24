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

#ifndef GUCEF_CORE_CDEFAULTCODECINSTANCE_H
#define GUCEF_CORE_CDEFAULTCODECINSTANCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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

/**
 *  The instance every codec can give: it gathers the input of a message until FINAL and then runs the codec's
 *  Encode / Decode with the parameters given at creation, plus "nonce" and "aad" from the call arguments.
 *  Works for any codec, but allocates per message. Codecs made for hot paths have instances of their own.
 */
class GUCEF_CORE_PUBLIC_CPP CDefaultCodecInstance : public CICodecInstance
{
    public:

    typedef CTSharedPtr< CICodec, MT::CMutex > TICodecPtr;

    CDefaultCodecInstance( const TICodecPtr& codec, const CValueList& params );

    virtual ~CDefaultCodecInstance() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Process( UInt32 flags                ,
                          const void* input           ,
                          UInt32 inputSize            ,
                          const SCodecCallArgs* args  ,
                          CDynamicBuffer& output      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TCodecStatus GetLastStatus( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDefaultCodecInstance( void );                                          /**< not implemented */
    CDefaultCodecInstance( const CDefaultCodecInstance& src );              /**< not implemented */
    CDefaultCodecInstance& operator=( const CDefaultCodecInstance& src );   /**< not implemented */

    void SetStatus( TCodecStatus status, const CString& message );

    private:

    TICodecPtr m_codec;
    CValueList m_params;
    CDynamicBuffer m_input;
    TCodecStatus m_lastStatus;
    CString m_lastErrorMessage;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDEFAULTCODECINSTANCE_H ? */
