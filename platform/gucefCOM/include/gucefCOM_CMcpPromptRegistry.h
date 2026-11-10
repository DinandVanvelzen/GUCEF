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

#ifndef GUCEF_COM_CMCPPROMPTREGISTRY_H
#define GUCEF_COM_CMCPPROMPTREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPREGISTRYBASE_H
#include "gucefCOM_CMcpRegistryBase.h"
#define GUCEF_COM_CMCPREGISTRYBASE_H
#endif /* GUCEF_COM_CMCPREGISTRYBASE_H ? */

#ifndef GUCEF_COM_CIMCPPROMPT_H
#include "gucefCOM_CIMcpPrompt.h"
#define GUCEF_COM_CIMCPPROMPT_H
#endif /* GUCEF_COM_CIMCPPROMPT_H ? */

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

/** A registered prompt, read only once registered */
class GUCEF_COM_PUBLIC_CPP CMcpPromptEntry
{
    public:

    CIMcpPromptPtr prompt;
    CMcpPromptDescriptor descriptor;
    CORE::CDataNode descriptorNode;         /**< the Prompt object of prompts/list */
    UInt64 generation;

    CMcpPromptEntry( void );
};

typedef CORE::CTSharedPtr< CMcpPromptEntry, MT::CMutex > CMcpPromptEntryPtr;

/*-------------------------------------------------------------------------*/

/** The prompts a server exposes, keyed by name in byte order. See CMcpRegistryBase for events and thread safety */
class GUCEF_COM_PUBLIC_CPP CMcpPromptRegistry : public CMcpRegistryBase
{
    public:

    static const CORE::CEvent PromptRegisteredEvent;        /**< TNameEventData: the name */
    static const CORE::CEvent PromptUnregisteredEvent;      /**< TNameEventData: the name */

    typedef GUCEF::vector< CMcpPromptEntryPtr > TPromptEntryVector;

    static void RegisterEvents( void );

    explicit CMcpPromptRegistry( const CORE::PulseGeneratorPtr& pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual ~CMcpPromptRegistry() GUCEF_VIRTUAL_OVERRIDE;

    /** Refused without a name, with arguments without a name or named twice, or when another prompt has the name */
    bool Register( const CIMcpPromptPtr& prompt, CORE::CString& error );

    bool Unregister( const CORE::CString& name );

    void UnregisterAll( void );

    bool TryGet( const CORE::CString& name, CMcpPromptEntryPtr& entry ) const;

    void GetEntries( TPromptEntryVector& entries ) const;

    UInt32 GetCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, CMcpPromptEntryPtr > TPromptEntryMap;

    TPromptEntryMap m_entries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPPROMPTREGISTRY_H ? */
