/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNTESTUTILS_H
#define GUCEF_DRN_TESTAPP_TESTDRNTESTUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

using namespace GUCEF;

/** A condition the pump helpers wait for */
class CTestCondition
{
    public:
    virtual ~CTestCondition() {}
    virtual bool IsMet( void ) = 0;
};

/** Pulses the global pulse generator until the condition is met or the time is up */
bool PumpUntil( CTestCondition& condition, UInt32 timeoutInMs = 10000 );

/** Pulses until counter >= expected */
bool PumpUntil( const UInt32& counter, UInt32 expected, UInt32 timeoutInMs = 10000 );

/** Pulses until the flag is set */
bool PumpUntilFlag( const bool& flag, UInt32 timeoutInMs = 10000 );

/** Pulses for the given time */
void PumpFor( UInt32 durationInMs );

/** A port for a test listener, every call another one */
UInt16 NextTestPort( void );

/** The web socket channel needs the sha1 codec */
bool EnsureHashCodecPluginLoaded( void );

/** Key agreement (x25519), signatures (ed25519) and the AEAD ciphers of codecspluginMONOCYPHER, for encrypted links */
bool EnsureCryptoCodecPluginLoaded( void );

/** An endpoint on loopback, for a channel type such as "tcp" or "lossy-udp", params appended as given ("?loss=0.1") */
DRN::CDRNEndpoint LoopbackEndpoint( const CORE::CString& channelType, UInt16 port, const CORE::CString& params = CORE::CString::Empty );

/*-------------------------------------------------------------------------*/

/** A message as a link delivered it */
class CRecordedMessage
{
    public:
    UInt8 lane;
    UInt8 msgClass;
    UInt32 opcode;
    CORE::CDynamicBuffer payload;
    CRecordedMessage( void ) : lane( 0 ), msgClass( 0 ), opcode( 0 ), payload() {}
};

/**
 *  Watches a node: its link events, and the messages of every link that becomes operational
 */
class CLinkRecorder : public CORE::CObserver
{
    public:

    CLinkRecorder( void );
    virtual ~CLinkRecorder();

    void WatchNode( DRN::CDRNNode& node );

    UInt32 operationalCount;
    UInt32 closedCount;
    UInt32 refusedCount;
    UInt32 latencyEventCount;
    UInt32 messageCount;
    DRN::TDRNLinkReason lastClosedReason;
    DRN::TDRNLinkReason lastRefusedReason;
    CORE::CString lastRefusedText;
    DRN::CDRNLatencyInfo lastLatencyInfo;
    DRN::TDRNPeerLinkVector links;
    GUCEF::vector< CRecordedMessage > messages;
    UInt32 eventThreadId;           /**< thread that delivered the last node event */

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** Two nodes linked to each other, a connecting one (a) and a listening one (b) */
class CLinkedPair
{
    public:

    CLinkedPair( void );
    ~CLinkedPair();

    /**
     *  Starts both nodes and links them
     *  @param listenEndpoint what node b listens on, node a connects to the same with connectParams appended
     */
    bool Setup( const DRN::CDRNNodeSettings& settingsA, const DRN::CDRNNodeSettings& settingsB, const DRN::CDRNEndpoint& listenEndpoint, const DRN::CDRNEndpoint& connectEndpoint, UInt32 timeoutInMs = 10000 );

    DRN::CDRNNode nodeA;
    DRN::CDRNNode nodeB;
    CLinkRecorder recorderA;
    CLinkRecorder recorderB;
    DRN::CDRNConnectorPtr connector;
    DRN::CDRNPeerLinkPtr linkA;
    DRN::CDRNPeerLinkPtr linkB;
};

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNTESTUTILS_H ? */
