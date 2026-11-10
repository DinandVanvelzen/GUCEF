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

#ifndef GUCEF_CORE_CINSTANCEBASEDCODEC_H
#define GUCEF_CORE_CINSTANCEBASEDCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

class CDynamicBuffer;

/*-------------------------------------------------------------------------*/

/**
 *  Base for codecs whose work is done by instances: the one shot Encode / Decode make an instance with the given
 *  parameters and run the input through it, the parameters "nonce" and "aad" becoming the call arguments.
 *  A derived codec only describes itself and makes instances.
 *
 *  Codecs with GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY get the whole input in one call, others get it in chunks.
 */
class GUCEF_CORE_PUBLIC_CPP CInstanceBasedCodec : public CICodec
{
    public:

    CInstanceBasedCodec( const CString& familyName ,
                         const CString& typeName   ,
                         UInt32 capabilities       );

    CInstanceBasedCodec( const CInstanceBasedCodec& src );

    virtual ~CInstanceBasedCodec() GUCEF_VIRTUAL_OVERRIDE;

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

    virtual bool CreateInstance( const CValueList& params       ,
                                 CICodecInstancePtr& instance   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The bytes of a parameter: a binary value or the bytes of a string (without the terminating zero). False when absent */
    static bool TryGetBinaryParam( const CValueList& params ,
                                   const char* name         ,
                                   const void*& data        ,
                                   UInt32& dataSize         );

    /** A parameter as an unsigned number, false when absent or not a number */
    static bool TryGetUInt32Param( const CValueList& params ,
                                   const char* name         ,
                                   UInt32& value            );

    /** The "output" parameter of hash codecs: true for "hex", false for "raw", 'defaultHex' when absent. False in 'isValid' for anything else */
    static bool IsHexOutputRequested( const CValueList& params ,
                                      bool defaultHex          ,
                                      bool& isValid            );

    /** Appends the bytes as lower case hexadecimal text */
    static void AppendHex( const UInt8* bytes        ,
                           UInt32 byteCount          ,
                           CDynamicBuffer& output    );

    protected:

    /** Makes an instance with the given parameters, or says why it cannot */
    virtual TCodecStatus CreateInstanceImpl( const CValueList& params       ,
                                             CICodecInstancePtr& instance   ,
                                             CString& errorMessage          ) = 0;

    private:

    CInstanceBasedCodec( void );                                        /**< not implemented */
    CInstanceBasedCodec& operator=( const CInstanceBasedCodec& src );   /**< not implemented */

    bool Run( bool encode               ,
              CIOAccess& source         ,
              CIOAccess& dest           ,
              const CValueList& params  );

    private:

    CString m_familyName;
    CString m_typeName;
    UInt32 m_capabilities;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CINSTANCEBASEDCODEC_H ? */
