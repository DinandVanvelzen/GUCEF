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

#ifndef GUCEF_CORE_CSOFTWARECRYPTOCODECS_H
#define GUCEF_CORE_CSOFTWARECRYPTOCODECS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CINSTANCEBASEDCODEC_H
#include "gucefCORE_CInstanceBasedCodec.h"
#define GUCEF_CORE_CINSTANCEBASEDCODEC_H
#endif /* GUCEF_CORE_CINSTANCEBASEDCODEC_H ? */

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
 *  CryptographicHashCodec "sha256" in portable code (CSha256).
 *  Parameter "output": "hex" (default) or "raw". Instances hash incrementally and allocate nothing.
 */
class GUCEF_CORE_PUBLIC_CPP CSha256Codec : public CInstanceBasedCodec
{
    public:

    static const CString TypeName;

    CSha256Codec( void );

    CSha256Codec( const CSha256Codec& src );

    virtual ~CSha256Codec() GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params       ,
                                             CICodecInstancePtr& instance   ,
                                             CString& errorMessage          ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSha256Codec& operator=( const CSha256Codec& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

/**
 *  MacCodec "hmac-sha256" in portable code (CHmacSha256).
 *  Parameter "key" (required). Encode gives the 32 byte MAC. Instances allocate nothing.
 */
class GUCEF_CORE_PUBLIC_CPP CHmacSha256Codec : public CInstanceBasedCodec
{
    public:

    static const CString TypeName;

    CHmacSha256Codec( void );

    CHmacSha256Codec( const CHmacSha256Codec& src );

    virtual ~CHmacSha256Codec() GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params       ,
                                             CICodecInstancePtr& instance   ,
                                             CString& errorMessage          ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CHmacSha256Codec& operator=( const CHmacSha256Codec& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

/**
 *  KeyDerivationCodec "hkdf-sha256" in portable code (CHkdfSha256).
 *  Parameters "salt", "info" (both optional) and "length" (default 32). Encode turns the input secret into the derived key.
 */
class GUCEF_CORE_PUBLIC_CPP CHkdfSha256Codec : public CInstanceBasedCodec
{
    public:

    static const CString TypeName;

    CHkdfSha256Codec( void );

    CHkdfSha256Codec( const CHkdfSha256Codec& src );

    virtual ~CHkdfSha256Codec() GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params       ,
                                             CICodecInstancePtr& instance   ,
                                             CString& errorMessage          ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CHkdfSha256Codec& operator=( const CHkdfSha256Codec& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSOFTWARECRYPTOCODECS_H ? */
