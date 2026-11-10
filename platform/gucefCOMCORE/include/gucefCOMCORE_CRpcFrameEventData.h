/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H
#define GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Event data of CIRpcTransport::FrameReceivedEvent: one whole frame and the protection class it arrived with.
 *
 *  A transport raises the event with data that links to its own receive buffer (no copy): the bytes are valid for
 *  the duration of the notification only. A clone (as made when the event is mailed to another thread) owns a copy.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcFrameEventData : public CORE::CICloneable
{
    public:

    /** Links to the bytes, which must stay valid while the event is delivered */
    CRpcFrameEventData( const void* frame, UInt32 size, TRpcProtectionClass protection );
    CRpcFrameEventData( const CRpcFrameEventData& src );
    virtual ~CRpcFrameEventData() GUCEF_VIRTUAL_OVERRIDE;

    const UInt8* GetFrame( void ) const;
    UInt32 GetSize( void ) const;
    TRpcProtectionClass GetProtection( void ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRpcFrameEventData& operator=( const CRpcFrameEventData& src );    /**< not implemented */

    const UInt8* m_frame;
    UInt32 m_size;
    TRpcProtectionClass m_protection;
    CORE::CDynamicBuffer m_ownedCopy;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCFRAMEEVENTDATA_H ? */
