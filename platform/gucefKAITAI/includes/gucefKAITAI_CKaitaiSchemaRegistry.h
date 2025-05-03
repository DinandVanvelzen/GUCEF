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

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#define GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTONREGISTRY_H
#include "CTONRegistry.h"
#define GUCEF_CORE_CTONREGISTRY_H
#endif /* GUCEF_CORE_CTONREGISTRY_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

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

class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaRegistry : public CORE::CTONRegistry< CORE::CTONRegistry< CKaitaiSchema, MT::CMutex >, MT::CMutex >
{
    public:

    typedef CORE::CTONRegistry< CKaitaiSchema, MT::CMutex >          TSchemaFamilyRegistry;
    typedef CORE::CTSharedPtr< TSchemaFamilyRegistry, MT::CMutex >   TSchemaFamilyRegistryPtr;
    typedef CORE::CTONRegistry< TSchemaFamilyRegistry, MT::CMutex >  TSchemaRegistryBase;        
    typedef TSchemaFamilyRegistry::TRegisteredObjPtr                 TSchemaPtr;
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CKaitaiSchemaPtr TryGetSchema( const CORE::CString& schemaFamily , 
                                   const CORE::CString& schemaName   ) const;

    CKaitaiSchemaBaseFieldPtr TryGetSchemaRootOrSubType( const CORE::CString& schemaFamily     , 
                                                         const CORE::CString& schemaId         ,
                                                         const CORE::CString& typeNameToSearch ) const;

    bool RegisterSchema( TSchemaPtr schema                 ,
                         const CORE::CString& schemaFamily );
    
    bool LoadSchemaFromString( const CORE::CString& schemaContent ,
                               const CORE::CString& schemaFamily  );

    bool LoadSchema( const CORE::CUri& schemaResource  ,
                     const CORE::CString& schemaFamily );

    bool LoadSchemas( const CORE::CUri& schemaResourceRoot ,
                      const CORE::CString& schemaFamily    ,
                      bool recursive                       );

    bool LoadSchemaUsingVfs( const CORE::CString& schemaResourcePath  ,
                             const CORE::CString& schemaFamily        );

    bool LoadSchemasUsingVfs( const CORE::CString& schemaResourcesRoot ,
                              const CORE::CString& schemaFamily        ,
                              bool recursive                           );

    virtual void Unregister( const CORE::CString& name ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void UnregisterAll( void ) GUCEF_VIRTUAL_OVERRIDE;    
    
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CKaitaiSchemaRegistry( void );

    virtual ~CKaitaiSchemaRegistry();

    private:
    
    typedef std::set< CKaitaiSchemaPtr >                TSchemaSet;
    typedef std::map< CORE::CString, TSchemaSet >       TSchemaSetMap;
    typedef std::map< CORE::CString, TSchemaSetMap >    TSchemaFamilyMap;

    void ResolveSchemaImportDependencies( CKaitaiSchemaPtr schema                   ,
                                          const CORE::CStringSet& unresolvedImports );

    CKaitaiSchemaRegistry( const CKaitaiSchemaRegistry& src );              /**< not implemented, don't use */
    CKaitaiSchemaRegistry& operator=( const CKaitaiSchemaRegistry& src );   /**< not implemented, don't use */

    private:

    TSchemaFamilyMap m_missingImportsSchemaFamilyMap; /**< The registry of schema families and their schemas */
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H ? */
