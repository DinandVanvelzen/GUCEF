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

#ifndef GUCEF_CORE_CCODECPARAMETERINFO_H
#define GUCEF_CORE_CCODECPARAMETERINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

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
 *  The description of one parameter of a codec, so applications and user interfaces can find out what a codec takes
 */
class GUCEF_CORE_PUBLIC_CPP CCodecParameterInfo
{
    public:

    CCodecParameterInfo( void );

    CCodecParameterInfo( const CString& name          ,
                         UInt8 typeId                 ,
                         bool isRequired              ,
                         const CVariant& defaultValue ,
                         const CString& description   );

    CCodecParameterInfo( const CCodecParameterInfo& src );

    ~CCodecParameterInfo();

    CCodecParameterInfo& operator=( const CCodecParameterInfo& src );

    void SetName( const CString& name );

    const CString& GetName( void ) const;

    /** GUCEF_DATATYPE_*: GUCEF_DATATYPE_BINARY_BLOB for keys and nonces */
    void SetTypeId( UInt8 typeId );

    UInt8 GetTypeId( void ) const;

    void SetIsRequired( bool isRequired );

    bool GetIsRequired( void ) const;

    /** What the codec uses when the parameter is not given, empty when there is nothing */
    void SetDefaultValue( const CVariant& defaultValue );

    const CVariant& GetDefaultValue( void ) const;

    void SetDescription( const CString& description );

    const CString& GetDescription( void ) const;

    private:

    CString m_name;
    UInt8 m_typeId;
    bool m_isRequired;
    CVariant m_defaultValue;
    CString m_description;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CCodecParameterInfo > CCodecParameterInfoList;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCODECPARAMETERINFO_H ? */
