/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H
#define GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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
 *  How a database client is opened (section 12 of the data tables design).
 *
 *  Serialized: backendName, database, readOnly, threadPoolName, bufferPoolMaxBytes and the backendOptions object.
 *  Not serialized, they are given by the code that opens the client: pulseGenerator and threadPool.
 */
class GUCEF_COMCORE_EXPORT_CPP CDatabaseClientSettings : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt64 DefaultBufferPoolMaxBytes = 512ULL * 1024 * 1024;

    CORE::CString backendName;                  /**< the name the backend is registered with in the factory, "vfstables" */
    CORE::CString database;                     /**< what the backend opens: for vfstables the VFS root path, for example "marketdata" */
    bool readOnly;                              /**< default false */
    CORE::CString threadPoolName;               /**< the pool of the async calls, default "DataTables" */
    UInt64 bufferPoolMaxBytes;                  /**< budget of the buffer pool of the client, default 512 MiB */
    CORE::CValueList backendOptions;            /**< backend specific */

    CORE::PulseGeneratorPtr pulseGenerator;     /**< events reach observers on its thread; the global one when not given */
    CORE::ThreadPoolPtr threadPool;             /**< used for the async calls instead of the pool of threadPoolName when given */

    CDatabaseClientSettings( void );

    CDatabaseClientSettings( const CDatabaseClientSettings& src );

    virtual ~CDatabaseClientSettings() GUCEF_VIRTUAL_OVERRIDE;

    CDatabaseClientSettings& operator=( const CDatabaseClientSettings& src );

    /** The serialized members are equal; pulseGenerator and threadPool are not compared */
    bool operator==( const CDatabaseClientSettings& other ) const;
    bool operator!=( const CDatabaseClientSettings& other ) const;

    /** A backend name, a database, a thread pool name and a buffer pool budget above zero */
    bool Validate( CORE::CString& error ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The settings are left as they were when the node does not hold valid settings */
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H ? */
