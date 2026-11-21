/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNMEMORYRESOURCESINK_H
#define GUCEF_DRN_CDRNMEMORYRESOURCESINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCESINK_H
#include "gucefDRN_CIDRNResourceSink.h"
#define GUCEF_DRN_CIDRNRESOURCESINK_H
#endif /* GUCEF_DRN_CIDRNRESOURCESINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Memory as the target of a transfer: the window that was asked for, at most maxBytes (TOO_LARGE otherwise, before
 *  anything is fetched). Not resumable. GetData() once the transfer completed.
 */
class GUCEF_DRN_EXPORT_CPP CDRNMemoryResourceSink : public CIDRNResourceSink
{
    public:

    static const UInt32 DefaultMaxBytes = 64 * 1024 * 1024;

    explicit CDRNMemoryResourceSink( UInt32 maxBytes = DefaultMaxBytes );

    virtual ~CDRNMemoryResourceSink() GUCEF_VIRTUAL_OVERRIDE;

    /** The window as it arrived; complete once the transfer completed */
    const CORE::CDynamicBuffer& GetData( void ) const;

    /** Takes the data out of the sink */
    void TakeData( CORE::CDynamicBuffer& destination );

    virtual TDRNResourceStatus Begin( const CORE::CString& provider       ,
                                      const CDRNResourceInfo& info        ,
                                      UInt64 windowOffset                 ,
                                      UInt64 windowLength                 ,
                                      bool resume                         ,
                                      CDRNResourcePartialState& state     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Write( UInt64 offset, const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus ReadBack( UInt64 offset, UInt32 size, void* destination ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Finish( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Abort( const CDRNResourcePartialState& state, bool discard ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNMemoryResourceSink( const CDRNMemoryResourceSink& src );               /**< not implemented */
    CDRNMemoryResourceSink& operator=( const CDRNMemoryResourceSink& src );    /**< not implemented */

    UInt32 m_maxBytes;
    UInt64 m_windowOffset;
    CORE::CDynamicBuffer m_data;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNMEMORYRESOURCESINK_H ? */
