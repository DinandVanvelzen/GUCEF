/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPJSONLINESAUDITLOG_H
#define GUCEF_COM_CMCPJSONLINESAUDITLOG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_COM_CMCPAUDITRECORD_H
#include "gucefCOM_CMcpAuditRecord.h"
#define GUCEF_COM_CMCPAUDITRECORD_H
#endif /* GUCEF_COM_CMCPAUDITRECORD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMcpServer;

/**
 *  An optional audit sink for applications that want a file and no code of their own: it subscribes to the AuditEvent
 *  of a CMcpServer and appends one JSON line per record (CMcpAuditRecord::ToJsonLine()) to a CIOAccess, which can be a
 *  file of the VFS. Lines end with a plain '\n', whatever the platform. With OpenRollingFile() the log rolls over to a
 *  new file at a size threshold and keeps a number of old files. Failures are logged and counted, and never reach the
 *  request: a log that cannot be written does not stop the server. The records hold no raw arguments, only their
 *  digest and a redacted preview. Tamper resistance is not a goal.
 *
 *  WriteRecord() is thread safe.
 */
class GUCEF_COM_PUBLIC_CPP CMcpJsonLinesAuditLog : public CORE::CObservingNotifier
{
    public:

    CMcpJsonLinesAuditLog( void );

    virtual ~CMcpJsonLinesAuditLog() GUCEF_VIRTUAL_OVERRIDE;

    /** Subscribes to the AuditEvent of the server, which has to outlive the attachment or call Detach() first */
    bool Attach( CMcpServer& server );

    void Detach( void );

    /** The destination of the lines: any open, writeable stream. Replaces an earlier destination */
    bool SetOutput( const CORE::IOAccessPtr& output );

    /**
     *  Appends to a file that rolls over to a new one at a size: "dir/audit.jsonl" is the file being written, the one it
     *  replaced is "dir/audit.1.jsonl", before that "dir/audit.2.jsonl" and so on. A record is never split over two
     *  files: the file rolls over before a record that would take it past the threshold. The oldest file goes when
     *  there are more than maxFiles. An existing file is appended to.
     *  @param rolloverBytes    the size at which a new file is started, at least 1
     *  @param maxFiles         the number of files to keep, the one being written included; between 2 and 1000
     */
    bool OpenRollingFile( const CORE::CString& path, UInt64 rolloverBytes, UInt32 maxFiles );

    /** Whether to flush the output after every record, which is the default: an audit trail that is lost in a buffer is of no use */
    void SetFlushEveryRecord( bool flushEveryRecord );

    /**
     *  Whether the files of OpenRollingFile() are for the user that runs the process only, which is the default: a
     *  new file is made that way (CORE::CreateOwnerOnlyFile(), never more open, whatever the umask), an existing one
     *  is restricted with CORE::SetOwnerOnlyAccess() when it is opened. Set before OpenRollingFile()
     */
    void SetRestrictToOwner( bool restrictToOwner );

    /** Appends one record. @return false when it could not be written; counted and logged */
    bool WriteRecord( const CMcpAuditRecord& record );

    void Flush( void );

    void Close( void );

    UInt64 GetRecordsWritten( void ) const;

    UInt64 GetWriteFailures( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CMcpJsonLinesAuditLog( const CMcpJsonLinesAuditLog& src );
    CMcpJsonLinesAuditLog& operator=( const CMcpJsonLinesAuditLog& src );

    bool OpenRollingFileUnlocked( void );

    /** Moves the files up one place and starts a new one. The lock is held */
    bool Rollover( void );

    void OnAuditEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    mutable MT::CMutex m_lock;
    CORE::IOAccessPtr m_output;
    bool m_flushEveryRecord;
    bool m_restrictToOwner;
    UInt64 m_recordsWritten;
    UInt64 m_writeFailures;
    CORE::CString m_rollingPath;                /**< empty when the output is a stream of the caller */
    UInt64 m_rolloverBytes;
    UInt32 m_maxFiles;
    UInt64 m_fileBytes;                         /**< what the file being written holds */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPJSONLINESAUDITLOG_H ? */
