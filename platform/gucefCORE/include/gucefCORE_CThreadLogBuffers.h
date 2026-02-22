/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CTHREADLOGBUFFERS_H
#define GUCEF_CORE_CTHREADLOGBUFFERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

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

/**
 *  Per-thread double buffer structure for logging.
 *  Each thread gets its own buffers to avoid contention.
 *  Internal implementation detail of CLogManager.
 */
class GUCEF_HIDDEN CThreadLogBuffers
{
    public:

    CThreadLogBuffers( void );

    ~CThreadLogBuffers();

    void Swap( void );

    CVariantStreamPtr GetFrontBuffer( void );

    CVariantStreamPtr GetBackBuffer( void );

    UInt32 GetThreadId( void ) const;

    void SetThreadId( UInt32 threadId );

    MT::CMutex& GetSwapLock( void );

    private:

    CThreadLogBuffers( const CThreadLogBuffers& src );              /**< not implemented, don't use */
    CThreadLogBuffers& operator=( const CThreadLogBuffers& src );   /**< not implemented, don't use */

    private:

    CVariantStreamPtr m_frontBuffer;    /**< Thread writes log entries here */
    CVariantStreamPtr m_backBuffer;     /**< Logger drains completed entries from here */
    MT::CMutex m_swapLock;              /**< Protects swap operation only */
    UInt32 m_threadId;                  /**< Thread ID owning these buffers */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTHREADLOGBUFFERS_H ? */
