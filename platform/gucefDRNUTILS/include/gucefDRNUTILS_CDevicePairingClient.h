/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#include "gucefDRNUTILS_CDeviceCredentials.h"
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The client side of device pairing (gucefASSETSTORE_RemoteNetworking_Design.md 3.4): with the one-time code a person
 *  typed, a DRN link without a pinned key to the pairing listener, authenticated by the shared secret derived from the
 *  code; Pairing/Begin, whose static key must be the one of the link's handshake and whose server proof (derived from
 *  the code) must verify before the key is trusted; a new Ed25519 device key and Pairing/Complete with its proof; the
 *  wait for the owner's confirmation when the window needs it; and the credentials of the device.
 *  Pair() is blocking and runs on the calling thread: when that thread drives the pulse generator given at construction
 *  (or nobody drives it, or none was given and the client made one of its own) it pumps it while it waits, otherwise it
 *  only waits for the thread that drives it. FingerprintEvent and AwaitingConfirmationEvent are raised during Pair()
 *  on that pulse generator. The code, the derived secret and the device seed are never logged.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDevicePairingClient : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent FingerprintEvent;             /**< CORE::TCloneableString: the fingerprint of the new device key, to compare with the owner's */
    static const CORE::CEvent AwaitingConfirmationEvent;    /**< the service waits for the owner's confirmation */

    static void RegisterEvents( void );

    enum EPairingFailure
    {
        PAIRINGFAILURE_NONE = 0 ,
        PAIRINGFAILURE_BAD_REQUEST ,        /**< the code, the endpoint or the device name is not usable */
        PAIRINGFAILURE_UNREACHABLE ,        /**< no pairing listener answered */
        PAIRINGFAILURE_WRONG_CODE ,         /**< the service refused the code (or the address is banned for wrong codes) */
        PAIRINGFAILURE_SERVER_PROOF ,       /**< the service could not prove it knows the code, or showed another key than its handshake: never trusted */
        PAIRINGFAILURE_REFUSED ,            /**< the service refused the pairing (window closed, revoked key, ...) */
        PAIRINGFAILURE_DECLINED ,           /**< the owner declined it */
        PAIRINGFAILURE_TIMEOUT ,            /**< no answer or no confirmation in time */
        PAIRINGFAILURE_STORE                /**< the credential store did not keep the credentials */
    };
    typedef enum EPairingFailure TPairingFailure;

    struct GUCEF_DRNUTILS_PUBLIC_CPP SRequest
    {
        CORE::CString pairingEndpoint;      /**< "tcp://host:port" of the pairing listener */
        CORE::CString code;                 /**< as typed */
        CORE::CString serviceName;          /**< the DRN service name of the service */
        CORE::CString drnSessionName;       /**< the DRN session of the session listener (the pairing session is this plus ".pair") */
        CORE::CString clientKind;
        CORE::CString deviceName;           /**< 1 to 64 characters */
        CORE::CString platform;
        CORE::CString appVersion;
        UInt32 connectTimeoutMs;            /**< default 20000 */
        UInt32 confirmationTimeoutMs;       /**< default 300000 */

        SRequest( void );
    };

    /** 'pulseGenerator' GUCEF_NULL: the client makes one of its own for each Pair() */
    explicit CDevicePairingClient( const CORE::PulseGeneratorPtr& pulseGenerator = CORE::PulseGeneratorPtr() );
    virtual ~CDevicePairingClient() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Pairs; true with the credentials. With a store they are kept in it under the slot (when 'slot' is not empty)
     *  before Pair() returns. False with the reason (GetLastFailure() says which kind)
     */
    bool Pair( const SRequest& request, CDeviceCredentials& credentials, CORE::CString& error,
               AUTH::ICredentialStore* store = GUCEF_NULL, const CORE::CString& slot = CORE::CString::Empty );

    TPairingFailure GetLastFailure( void ) const;

    /** The fingerprint of the device key of the last Pair() */
    const CORE::CString& GetFingerprint( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDevicePairingClient( const CDevicePairingClient& src );                /**< not implemented */
    CDevicePairingClient& operator=( const CDevicePairingClient& src );     /**< not implemented */

    bool Fail( TPairingFailure failure, const CORE::CString& why, CORE::CString& error );
    static void PumpStep( const CORE::PulseGeneratorPtr& pulseGenerator );

    CORE::PulseGeneratorPtr m_pulse;
    TPairingFailure m_lastFailure;
    CORE::CString m_fingerprint;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCLIENT_H ? */
