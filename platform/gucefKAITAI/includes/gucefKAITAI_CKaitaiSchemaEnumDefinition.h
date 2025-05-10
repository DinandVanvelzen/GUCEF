/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#ifndef GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H
#define GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#include "gucefKAITAI_CKaitaiSchemaBaseField.h"
#define GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/


class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaEnumDefinition : public CKaitaiSchemaBaseField ,
                                                            public CORE::CTSharedObjCreator< CKaitaiSchemaEnumDefinition, MT::CMutex >
{
    public:

    typedef std::pair< const Int32, CORE::CString >                                                             TInt32AndStringPair;
    typedef std::map< Int32, CORE::CString, std::less< Int32 >, gucef_allocator< TInt32AndStringPair > >        TInt32ToStringMap;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaEnumDefinition, MT::CMutex >::TBasicSharedPtrType   CKaitaiSchemaEnumDefinitionPtr;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaEnumDefinition, MT::CMutex >::TSharedPtrType        CKaitaiSchemaEnumDefinitionTypedPtr;

    static const CORE::CString ClassTypeName;

    CKaitaiSchemaEnumDefinition( void );                                    /**< dont use this, use the other constructor */
    CKaitaiSchemaEnumDefinition( CKaitaiSchemaBaseFieldPtr parent );
    CKaitaiSchemaEnumDefinition( const CKaitaiSchemaEnumDefinition& src );   
    CKaitaiSchemaEnumDefinition& operator=( const CKaitaiSchemaEnumDefinition& src ); 

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    const TInt32ToStringMap& GetEnumValues( void ) const;
    
    private:

    TInt32ToStringMap m_enumValues;
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaEnumDefinition::CKaitaiSchemaEnumDefinitionPtr         CKaitaiSchemaEnumDefinitionPtr;
typedef CKaitaiSchemaEnumDefinition::CKaitaiSchemaEnumDefinitionTypedPtr    CKaitaiSchemaEnumDefinitionTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H ? */
