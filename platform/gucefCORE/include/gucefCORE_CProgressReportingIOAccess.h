/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H
#define GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H

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
 *  Receiver of the progress reports of a CProgressReportingIOAccess
 */
class GUCEF_CORE_PUBLIC_CPP CIIOProgressObserver
{
    public:

    CIIOProgressObserver( void );

    virtual ~CIIOProgressObserver();

    /**
     *  Called on the thread doing the I/O after data was read or written, subject to the report interval
     *
     *  @param totalBytesRead       Total number of bytes read through the access object so far
     *  @param totalBytesWritten    Total number of bytes written through the access object so far
     *  @return true to continue, false to abort. After an abort all further reads and writes fail,
     *          which makes a blocking operation on top of the access object end early.
     */
    virtual bool OnIOProgress( UInt64 totalBytesRead    ,
                               UInt64 totalBytesWritten ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  I/O access decorator which counts the bytes read and written through it and reports progress to an observer.
 *  It does not own the access object it wraps, which must outlive the decorator.
 *
 *  This is how long running blocking operations that take a CIOAccess, such as downloading a resource into a file,
 *  can report progress and can be aborted from the outside without the operation having to know about either.
 *  Not thread-safe, like most CIOAccess implementations: use it from the thread doing the I/O.
 */
class GUCEF_CORE_PUBLIC_CPP CProgressReportingIOAccess : public CIOAccess
{
    public:

    /**
     *  @param wrappedAccess            The access object to which all I/O is passed on
     *  @param observer                 Observer to report to, if any. Must outlive the decorator.
     *  @param reportIntervalInBytes    Minimal number of bytes transferred between reports, 0 reports after every
     *                                  read or write. A larger interval keeps the overhead for fast transfers low.
     */
    CProgressReportingIOAccess( CIOAccess& wrappedAccess                  ,
                                CIIOProgressObserver* observer = GUCEF_NULL ,
                                UInt32 reportIntervalInBytes = 0          );

    virtual ~CProgressReportingIOAccess() GUCEF_VIRTUAL_OVERRIDE;

    UInt64 GetTotalBytesRead( void ) const;

    UInt64 GetTotalBytesWritten( void ) const;

    /**
     *  Aborts all further I/O, or allows it again
     */
    void SetAborted( bool aborted );

    bool IsAborted( void ) const;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString ReadLine( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString ReadString( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Read( void *dest      ,
                         UInt32 esize    ,
                         UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Write( const void* srcdata ,
                          UInt32 esize        ,
                          UInt32 elements     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 Seek( Int64 offset ,
                        Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 Setpos( UInt64 position ) GUCEF_VIRTUAL_OVERRIDE;

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** a clone wraps the same access object and observer, with its own counters */
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CProgressReportingIOAccess( void );
    CProgressReportingIOAccess& operator=( const CProgressReportingIOAccess& src );

    void Report( UInt64 bytesJustTransferred );

    private:

    CIOAccess* m_wrappedAccess;
    CIIOProgressObserver* m_observer;
    UInt32 m_reportIntervalInBytes;
    UInt64 m_totalBytesRead;
    UInt64 m_totalBytesWritten;
    UInt64 m_bytesSinceLastReport;
    bool m_aborted;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPROGRESSREPORTINGIOACCESS_H ? */
