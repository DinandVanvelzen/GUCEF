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

#ifndef GUCEF_DRN_CDRNIOACCESSRESOURCESINK_H
#define GUCEF_DRN_CDRNIOACCESSRESOURCESINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

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
 *  A CORE::CIOAccess as the target of a transfer: the window is written into it in order, as a stream, from threads
 *  of the pool. The access is the application's: it outlives the transfer and is not used by others meanwhile.
 *  Not resumable.
 */
class GUCEF_DRN_EXPORT_CPP CDRNIOAccessResourceSink : public CIDRNResourceSink
{
    public:

    explicit CDRNIOAccessResourceSink( CORE::CIOAccess& access );

    virtual ~CDRNIOAccessResourceSink() GUCEF_VIRTUAL_OVERRIDE;

    /** Bytes written into the access */
    UInt64 GetBytesWritten( void ) const;

    virtual TDRNResourceStatus Begin( const CORE::CString& provider       ,
                                      const CDRNResourceInfo& info        ,
                                      UInt64 windowOffset                 ,
                                      UInt64 windowLength                 ,
                                      bool resume                         ,
                                      CDRNResourcePartialState& state     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Write( UInt64 offset, const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Finish( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Abort( const CDRNResourcePartialState& state, bool discard ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsInOrder( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNIOAccessResourceSink( const CDRNIOAccessResourceSink& src );               /**< not implemented */
    CDRNIOAccessResourceSink& operator=( const CDRNIOAccessResourceSink& src );    /**< not implemented */

    CORE::CIOAccess* m_access;
    UInt64 m_nextOffset;
    UInt64 m_bytesWritten;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNIOACCESSRESOURCESINK_H ? */
