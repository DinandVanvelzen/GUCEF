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

#ifndef GUCEF_CORE_CBUILTINCODECS_H
#define GUCEF_CORE_CBUILTINCODECS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

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

class CCoreGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  The codecs gucefCORE brings itself: portable software codecs (always) and the crypto facilities of the
 *  operating system (switchable). Registered in the codec registry at start up, taken out at shut down.
 *
 *  Config: node "BuiltInCodecs", attribute "registerOsCryptoCodecs" (default true).
 */
class GUCEF_CORE_PUBLIC_CPP CBuiltInCodecs : public CGloballyConfigurable
{
    public:

    typedef CCodecRegistry::TICodecPtrVector TICodecPtrVector;

    static const CString ClassTypeName;

    /** Registers or takes out the OS crypto codecs */
    void SetRegisterOsCryptoCodecs( bool registerOsCryptoCodecs );

    bool GetRegisterOsCryptoCodecs( void ) const;

    /** The OS crypto codecs that are registered */
    void GetOsCryptoCodecs( TICodecPtrVector& codecs ) const;

    /** The software codecs that are registered */
    void GetSoftwareCodecs( TICodecPtrVector& codecs ) const;

    virtual bool SaveConfig( CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CBuiltInCodecs( CCodecRegistry& registry );

    virtual ~CBuiltInCodecs() GUCEF_VIRTUAL_OVERRIDE;

    /** Registers the software codecs and, when switched on, the OS ones */
    void RegisterCodecs( void );

    /** Takes all of them out of the registry */
    void UnregisterCodecs( void );

    void RegisterOsCryptoCodecs( void );

    void UnregisterOsCryptoCodecs( void );

    private:

    CBuiltInCodecs( void );                                     /**< not implemented */
    CBuiltInCodecs( const CBuiltInCodecs& src );                /**< not implemented */
    CBuiltInCodecs& operator=( const CBuiltInCodecs& src );     /**< not implemented */

    private:

    CCodecRegistry* m_registry;
    TICodecPtrVector m_softwareCodecs;
    TICodecPtrVector m_osCodecs;
    bool m_registerOsCryptoCodecs;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CBUILTINCODECS_H ? */
