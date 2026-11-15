/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H
#define GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_WEB_IWEBVIEWENGINE_H
#include "gucefWEB_IWebViewEngine.h"
#define GUCEF_WEB_IWEBVIEWENGINE_H
#endif /* GUCEF_WEB_IWEBVIEWENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The engines that are there, by name, in the order they were registered. Plugins register their engine when they
 *  load (in plugin load order: groups in ini order, plugins of a group in order, GroupPriority and PriorityInGroup
 *  overriding the implied place, as CORE::CPluginControl loads them) and unregister it when they unload. That order is
 *  the order of preference unless the application sets one (CWebViewGlobalSettings::engineOrder).
 *  Registration only stores the engine: no thread, no engine library call. Thread safe.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewEngineRegistry : public CORE::CNotifier
{
    public:

    static const CORE::CEvent EngineRegisteredEvent;        /**< TCloneableString: the name of the engine */
    static const CORE::CEvent EngineUnregisteredEvent;      /**< TCloneableString: the name of the engine */

    static void RegisterEvents( void );

    typedef GUCEF::vector< TWebViewEnginePtr > TEngineVector;

    CWebViewEngineRegistry( void );

    virtual ~CWebViewEngineRegistry();

    /** False for a NULL engine, an engine of another interface version, or a name that is registered already */
    bool Register( const TWebViewEnginePtr& engine );

    /** False when no engine of that name is registered */
    bool Unregister( const CString& engineName );

    bool IsRegistered( const CString& engineName ) const;

    /** The engine of a name (case is ignored); NULL when there is none */
    TWebViewEnginePtr GetEngine( const CString& engineName ) const;

    /** The names in the order of registration */
    CString::StringVector GetEngineNames( void ) const;

    /** The engines in the order of registration */
    TEngineVector GetEngines( void ) const;

    /**
     *  The engines in the order of preference: first those named in 'preferredOrder' (in that order, names of engines
     *  that are not registered are skipped), then the others in the order of registration
     */
    TEngineVector GetEnginesInOrder( const CString::StringVector& preferredOrder ) const;

    UInt32 GetEngineCount( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWebViewEngineRegistry( const CWebViewEngineRegistry& src );                /**< not implemented */
    CWebViewEngineRegistry& operator=( const CWebViewEngineRegistry& src );     /**< not implemented */

    Int32 Find( const CString& engineName ) const;

    private:

    mutable MT::CMutex m_lock;
    TEngineVector m_engines;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H ? */
