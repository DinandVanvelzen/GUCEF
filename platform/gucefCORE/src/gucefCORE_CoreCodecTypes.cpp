/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCORE_CoreCodecTypes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CoreCodecTypes::HashingCodec            = "HashingCodec";
const CORE::CString CoreCodecTypes::CryptographicHashCodec  = "CryptographicHashCodec";
const CORE::CString CoreCodecTypes::ChecksumCodec           = "ChecksumCodec";
const CORE::CString CoreCodecTypes::CompressionCodec        = "CompressionCodec";
const CORE::CString CoreCodecTypes::EncryptionCodec         = "EncryptionCodec";
const CORE::CString CoreCodecTypes::MacCodec                = "MacCodec";
const CORE::CString CoreCodecTypes::KeyDerivationCodec      = "KeyDerivationCodec";
const CORE::CString CoreCodecTypes::KeyAgreementCodec       = "KeyAgreementCodec";
const CORE::CString CoreCodecTypes::SignatureCodec          = "SignatureCodec";

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace CORE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
