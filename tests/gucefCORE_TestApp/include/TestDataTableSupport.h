/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLESUPPORT_H
#define GUCEF_CORE_TESTAPP_TESTDATATABLESUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CORETEST {

/** The DataNode codec of the type, loading its plugin from the module directory when it is not registered yet */
inline bool
GetDataTableTestCodec( const char* codecType, const char* pluginName, GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{
    GUCEF::CORE::CCoreGlobal* coreGlobal = GUCEF::CORE::CCoreGlobal::Instance();
    if ( coreGlobal->GetDStoreCodecRegistry().TryLookup( codecType, codec, false ) )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    GUCEF::CORE::CString firstPluginPath = GUCEF::CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    GUCEF::CORE::CString secondPluginPath = GUCEF::CORE::CString( "$MODULEDIR$/" ) + pluginName;
    #else
    GUCEF::CORE::CString firstPluginPath = GUCEF::CORE::CString( "$MODULEDIR$/" ) + pluginName;
    GUCEF::CORE::CString secondPluginPath = GUCEF::CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    #endif

    GUCEF::CORE::CPluginControl& pluginControl = coreGlobal->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );

    return coreGlobal->GetDStoreCodecRegistry().TryLookup( codecType, codec, false );
}

/** The text a buffer holds */
inline GUCEF::CORE::CString
DataTableTestBufferText( const GUCEF::CORE::CDynamicBuffer& buffer )
{
    return GUCEF::CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
}

/**
 *  A schema with one column of every type: b bool, i8 .. u64, f32, f64, dec decimal(18,4), d date,
 *  ts timestamp(us,utc), tms timestamp(ms), tns timestamp(ns,utc), s utf8, bin binary. All nullable.
 */
inline GUCEF::CORE::CDataTableSchemaPtr
MakeAllTypesSchema( bool nullable = true )
{
    using namespace GUCEF::CORE;
    CDataTableSchema schema;
    schema.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_BOOL, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i8", DATATABLE_TYPE_INT8, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i16", DATATABLE_TYPE_INT16, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i32", DATATABLE_TYPE_INT32, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "i64", DATATABLE_TYPE_INT64, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u8", DATATABLE_TYPE_UINT8, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u16", DATATABLE_TYPE_UINT16, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u32", DATATABLE_TYPE_UINT32, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "u64", DATATABLE_TYPE_UINT64, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "f64", DATATABLE_TYPE_FLOAT64, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Decimal( "dec", 18, 4, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "d", DATATABLE_TYPE_DATE, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Timestamp( "tms", DATATABLE_TIMEUNIT_MILLISECONDS, false, nullable ) );
    schema.AddColumn( CDataTableColumnDef::Timestamp( "tns", DATATABLE_TIMEUNIT_NANOSECONDS, true, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8, nullable ) );
    schema.AddColumn( CDataTableColumnDef( "bin", DATATABLE_TYPE_BINARY, nullable ) );
    return CreateDataTableSchemaPtr( schema );
}

/** True when the pointer lies within the data and padding of the buffer */
inline bool
PointsIntoDataTableBuffer( const void* pointer, const GUCEF::CORE::CDataTableBuffer& buffer )
{
    const char* begin = static_cast< const char* >( buffer.GetData() );
    const char* end = begin + buffer.GetCapacity() + GUCEF::CORE::DataTableBufferPadding;
    const char* p = static_cast< const char* >( pointer );
    return p >= begin && p < end;
}

}; /* namespace CORETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLESUPPORT_H ? */
