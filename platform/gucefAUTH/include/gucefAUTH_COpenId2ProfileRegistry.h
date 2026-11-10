/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_COPENID2PROFILEREGISTRY_H
#define GUCEF_AUTH_COPENID2PROFILEREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#include "gucefAUTH_IOpenId2ProviderProfile.h"
#define GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H
#endif /* GUCEF_AUTH_IOPENID2PROVIDERPROFILE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The profiles of the OpenID 2.0 providers known to the process, by provider name (the name of the descriptor they belong
 *  to). Empty when the module is set up: plugins add theirs when they load and take them away when they unload, see
 *  IOpenId2ProviderProfile. The profiles are not owned. Can be used from several threads.
 */
class GUCEF_AUTH_EXPORT_CPP COpenId2ProfileRegistry
{
    public:

    COpenId2ProfileRegistry( void );

    ~COpenId2ProfileRegistry();

    /** Adds the profile under its provider name; false when it is null, has no name or the name has a profile already */
    bool Register( IOpenId2ProviderProfile* profile );

    /** Takes the profile of that name away; false when there is none. Not while a login or a verification with it runs */
    bool Unregister( const CORE::CString& providerName );

    /** The profile of the provider, GUCEF_NULL when there is none */
    IOpenId2ProviderProfile* Find( const CORE::CString& providerName ) const;

    UInt32 GetCount( void ) const;

    void Clear( void );

    private:

    COpenId2ProfileRegistry( const COpenId2ProfileRegistry& src );              /**< not implemented */
    COpenId2ProfileRegistry& operator=( const COpenId2ProfileRegistry& src );   /**< not implemented */

    private:

    typedef GUCEF::map< CORE::CString, IOpenId2ProviderProfile* > TProfileMap;

    mutable MT::CMutex m_lock;
    TProfileMap m_profiles;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_COPENID2PROFILEREGISTRY_H ? */
