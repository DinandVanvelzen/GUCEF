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

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHER_H
#define GUCEF_DRN_CDRNTABLEPUBLISHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <utility>

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H
#include "gucefDRN_CDRNPublishedTableSchema.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLESCHEMA_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H
#include "gucefDRN_CDRNPublishedTableMeta.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHERSETTINGS_H
#include "gucefDRN_CDRNTablePublisherSettings.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHERSETTINGS_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHERSETTINGS_H ? */

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
 *  Publishes a typed row set (a page of a server side view model) into a reliable grid of the local member, for
 *  CDRNPublishedTable on every replica. Reserved grid keys: "#schema" (CDRNPublishedTableSchema) and "#meta"
 *  (CDRNPublishedTableMeta); every other key is a row, its value one CVariantStream segment: version, the page revision
 *  in which the row last changed, the cells in schema order without the key column.
 *
 *  A client of the public API of the grid only: nothing is added to the grid, the replicator or the wire. A row whose
 *  cells did not change is not written, so republishing a whole row set sends only the rows that changed.
 *  Updates: BeginUpdate() / EndUpdate() (CDRNTablePublisherUpdateScope); EndUpdate() writes #meta when something
 *  changed. SetRow(), RemoveRow() and PublishRows() called outside an update are an update of their own (one page
 *  revision each), so that #meta always counts what the rows say. SetScalar(), SetSourceVersion() and SetSchema()
 *  between updates go with the next update, which then writes #meta even without a row change; BeginUpdate() never
 *  drops what is not published yet.
 *
 *  Threads: every call takes the node lock for what it changes. An update belongs to the thread that opened it, and
 *  holds the node lock until it ends: on another thread SetRow(), RemoveRow(), PublishRows() and PublishFrom() are
 *  outside an update (they open one of their own, which waits for the one that is open), and EndUpdate() fails.
 */
class GUCEF_DRN_EXPORT_CPP CDRNTablePublisher
{
    public:

    static const char* SchemaKey;           /**< "#schema" */
    static const char* MetaKey;             /**< "#meta" */
    static const UInt8 RowEncodingVersion = 1;

    typedef std::pair< CORE::CVariant, CORE::CVariant::VariantVector > TKeyedRow;
    typedef GUCEF::vector< TKeyedRow > TKeyedRows;

    /** The grid must be local, reliable and empty, else IsValid() is false and every call fails */
    CDRNTablePublisher( CDRNDataGrid& grid, const CDRNTablePublisherSettings& settings );

    ~CDRNTablePublisher();

    bool IsValid( void ) const;

    /** Writes #schema; between updates or inside one, the application republishes the rows itself */
    bool SetSchema( const CDRNPublishedTableSchema& schema );

    const CDRNPublishedTableSchema& GetSchema( void ) const;

    /** Opens an update: holds the node lock, so that the writes of the update fall in one tick; keeps what is pending */
    bool BeginUpdate( void );

    /** Closes the update of this thread: writes #meta with the next page revision when anything changed */
    bool EndUpdate( void );

    /**
     *  One row: the cells in schema order without the key column; false when refused (logged once per key, counted).
     *  Outside an update the row is an update of its own.
     */
    bool SetRow( const CORE::CVariant& key, const CORE::CVariant::VariantVector& cells );

    /** Removes a row; outside an update the removal is an update of its own */
    bool RemoveRow( const CORE::CVariant& key );

    /**
     *  The whole row set of the new version: every given row is set, every published key not given is removed.
     *  Outside an update the row set is one update of its own.
     */
    bool PublishRows( const TKeyedRows& rows );

    /**
     *  As PublishRows() with the rows of a scan of the table (sync, on the calling thread). The published columns are
     *  found in the table by their data table column name (CDRNPublishedColumn::def.name), the key column among them;
     *  the query selects the rows (and may narrow the projection, as long as every published column stays in it).
     *  Called outside an update, PublishFrom() reads the rows first and opens an update of its own once they are read;
     *  inside one, the rows go into that update. The scan subscribes to the pulse generator of the table. Where the
     *  calling thread holds the node lock while it scans, inside an update and on a thread that works on the node (a
     *  handler of a pulse of the node's pulse generator, CDRNNode::IsWorkingOnThisThread()), only the thread that pulses
     *  that pulse generator scans (other threads get false), so that no pulse of it waits for the node lock this thread
     *  holds. A thread that holds the node lock otherwise (taken through CDRNNode::GetLock()) does not call it.
     */
    bool PublishFrom( CORE::CIDataTable& table, const CORE::CDataTableQuery& query );

    /** A header scalar of the page, written with #meta at the end of the update */
    void SetScalar( const CORE::CString& name, const CORE::CVariant& value );

    /** The application's version of the data the rows come from, written with #meta */
    void SetSourceVersion( UInt64 sourceVersion );

    UInt64 GetPageRevision( void ) const;
    UInt32 GetRowCount( void ) const;
    UInt64 GetOversizeRowCount( void ) const;
    UInt64 GetTypeMismatchCount( void ) const;
    UInt64 GetReservedKeyCount( void ) const;

    /** Whether the key is one of the reserved ones (text starting with '#') */
    static bool IsReservedKey( const CORE::CVariant& key );

    private:

    CDRNTablePublisher( const CDRNTablePublisher& src );
    CDRNTablePublisher& operator=( const CDRNTablePublisher& src );

    bool WriteValue( const CORE::CVariant& key, const CORE::CVariantStream& stream );
    bool CheckCells( const CORE::CVariant& key, const CORE::CVariant::VariantVector& cells );
    void NoteRefusal( const CORE::CVariant& key, const char* why );

    /** Whether the calling thread has an update open (BeginUpdate()), which holds the node lock */
    bool IsInUpdateOnThisThread( void ) const;

    MT::CMutex& GetNodeLock( void ) const;

    typedef GUCEF::set< CORE::CVariant > TKeySet;

    CDRNDataGrid* m_grid;
    CDRNTablePublisherSettings m_settings;
    bool m_valid;
    CDRNPublishedTableSchema m_schema;
    bool m_haveSchema;
    CORE::CDataTableSchemaPtr m_dataSchema;
    CORE::CDataTableBatchBuilder* m_checker;
    CDRNPublishedTableMeta m_meta;
    TKeySet m_liveKeys;
    TKeySet m_changedKeys;              /**< rows written in this update */
    TKeySet m_refusedKeys;              /**< logged once */
    UInt32 m_removedInUpdate;
    bool m_metaChanged;
    UInt32 m_updateDepth;               /**< of the thread in m_updateThreadId, under the node lock */
    MT::CTAtomic< UInt32 > m_updateThreadId;    /**< the thread that has the update open, 0 when none is */
    UInt32 m_rowPrefixSize;             /**< bytes of version and revision in front of the cells */
    CORE::CVariantStream m_stream;
    CORE::CVariant m_existing;
    UInt64 m_oversizeRows;
    UInt64 m_typeMismatches;
    UInt64 m_reservedKeys;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNTABLEPUBLISHER_H ? */
