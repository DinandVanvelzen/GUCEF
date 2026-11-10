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

#ifndef GUCEF_CORE_CPROCESSPIPE_H
#define GUCEF_CORE_CPROCESSPIPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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
 *  One end of an anonymous pipe to a child process, as a CIOAccess: the end that this process holds of the standard
 *  input, output or error of a child that CProcess::LaunchWithPipes() started.
 *
 *  A pipe is a stream: it has no position and cannot seek. Read() blocks until the requested bytes are in or the
 *  child closed its end; ReadSome() and WaitForData() are for callers that must not wait for ever. Writing to a child
 *  that has gone gives an error and never a signal. The pipe closes its handle when closed or destroyed. Not thread safe:
 *  use one pipe from one thread at a time, except that GetNativeHandle() may be handed to the reader and writer
 *  classes of this library for the pipe's lifetime.
 */
class GUCEF_CORE_PUBLIC_CPP CProcessPipe : public CIOAccess
{
    public:

    /** The native handle of a pipe end: a file descriptor, or on MS Windows a HANDLE */
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    typedef void* TNativeHandle;
    #else
    typedef int TNativeHandle;
    #endif

    enum EDirection
    {
        PIPE_READ_FROM_CHILD    ,   /**< the standard output or error of the child */
        PIPE_WRITE_TO_CHILD         /**< the standard input of the child */
    };

    /** The value of a handle that is not open */
    static TNativeHandle GetInvalidHandle( void );

    /** Takes over the handle, which is closed again by Close() and by the destructor */
    CProcessPipe( TNativeHandle handle, EDirection direction );

    virtual ~CProcessPipe() GUCEF_VIRTUAL_OVERRIDE;

    EDirection GetDirection( void ) const;

    /** For handing to CStandardInputReader and CStandardOutputWriter. The pipe keeps ownership */
    TNativeHandle GetNativeHandle( void ) const;

    /**
     *  Waits until a Read() will not block: data is there, or the other end closed. Only for pipes that read.
     *  @return true when there is something to read or the end of the stream was reached, false at the timeout
     */
    bool WaitForData( UInt32 timeoutInMs );

    /**
     *  Reads what is there, blocking until at least one byte arrives or the other end closed.
     *  @return the number of bytes, 0 at the end of the stream or on error
     */
    UInt32 ReadSome( void* dest, UInt32 maxBytes );

    // CIOAccess

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads until all bytes are in, the other end closed or an error happened. @return the number of elements read */
    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    /** Writes all of it, or stops at an error. @return the number of elements written */
    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    using CIOAccess::Write;

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** True once a read found the other end closed */
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** A pipe end cannot be copied: returns GUCEF_NULL */
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CProcessPipe( const CProcessPipe& src );
    CProcessPipe& operator=( const CProcessPipe& src );

    private:

    TNativeHandle m_handle;
    EDirection m_direction;
    bool m_atEnd;
    UInt64 m_bytesTransferred;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPROCESSPIPE_H ? */
