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

#ifndef GUCEF_KAITAI_CKAITAIREADER_H
#define GUCEF_KAITAI_CKAITAIREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMA_H
#include "gucefKAITAI_CKaitaiSchema.h"
#define GUCEF_KAITAI_CKAITAISCHEMA_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMA_H ? */

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


class GUCEF_KAITAI_PUBLIC_CPP CKaitaiReader : public CORE::CTSharedObjCreator< CKaitaiReader, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;


    CKaitaiReader( void );
    CKaitaiReader( const CORE::CString& schemaFamily );
    CKaitaiReader( const CKaitaiReader& src );   
    virtual ~CKaitaiReader() GUCEF_VIRTUAL_OVERRIDE;
    CKaitaiReader& operator=( const CKaitaiReader& src );    

    bool ConstructExecutionPlan( CKaitaiSchemaPtr schema );

    private:

};

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAIREADER_H ? */
