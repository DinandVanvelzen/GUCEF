/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CIMAPBODYSTRUCTURE_H
#define GUCEF_COM_CIMAPBODYSTRUCTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAPVALUE_H
#include "gucefCOM_CImapValue.h"
#define GUCEF_COM_CIMAPVALUE_H
#endif /* GUCEF_COM_CIMAPVALUE_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The BODYSTRUCTURE of a message (RFC 3501 section 7.4.2): the parts a message has without having to fetch it */
class GUCEF_COM_PUBLIC_CPP CImapBodyStructure
{
    public:

    bool isMultipart;
    CORE::CString type;                         /**< lower case: text, image, application, multipart */
    CORE::CString subtype;                      /**< lower case */
    CMimeCodec::TParamVector params;
    CORE::CString contentId;
    CORE::CString description;
    CORE::CString encoding;                     /**< lower case */
    UInt32 size;                                /**< octets, not for multiparts */
    UInt32 lines;                               /**< text parts */
    CORE::CString disposition;                  /**< lower case: attachment or inline */
    CMimeCodec::TParamVector dispositionParams;
    GUCEF::vector< CImapBodyStructure* > children;  /**< owned: the parts of a multipart */

    CImapBodyStructure( void );
    CImapBodyStructure( const CImapBodyStructure& src );
    ~CImapBodyStructure();
    CImapBodyStructure& operator=( const CImapBodyStructure& src );

    /** type/subtype */
    CORE::CString GetContentType( void ) const;

    /** The file name of the part, from the disposition or the type parameters, empty when it has none */
    CORE::CString GetFileName( void ) const;

    void Clear( void );

    static bool FromValue( const CImapValue& value, CImapBodyStructure& structure );

    private:

    void CopyFrom( const CImapBodyStructure& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPBODYSTRUCTURE_H ? */
