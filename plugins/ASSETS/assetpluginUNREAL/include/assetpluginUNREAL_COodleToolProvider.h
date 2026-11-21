/*
 *  assetpluginUNREAL: The Oodle provider that asks an external tool in a child process
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H
#define GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#include "assetpluginUNREAL_CIUnrealOodleProvider.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decompresses blocks with the external tool "ooz_extract" (a separate GPL licensed program, never linked: only started). One
 *  child process in batch mode lives from the first block to Stop() or the destructor. The compressed bytes of a block go to a
 *  file in a folder of the per-user cache, the request is one line on the standard input of the child ("ok <bytes>" or
 *  "error <code> <message>" comes back), and the decompressed bytes are read from an output file. Files of a request are removed
 *  when it is done, the folder when the provider stops.
 *
 *  A crashed or hung up child (the end of its answers, a write that fails) is replaced once per request, then the request
 *  fails. A request that takes longer than the timeout kills the child and fails. A request for cancellation (the report that
 *  SetCancellationSource() names) is polled while waiting; it kills the child and fails the request.
 *
 *  Windows: the same code is used with the pipes of CProcess; it is compiled but not run on that platform yet.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP COodleToolProvider : public CIUnrealOodleProvider
{
    public:

    /** The tool at the place CUnrealCapabilities names */
    COodleToolProvider( void );

    /** A tool at the given path (tests, hosts that know better) */
    explicit COodleToolProvider( const CORE::CString& toolPath );

    virtual ~COodleToolProvider() GUCEF_VIRTUAL_OVERRIDE;

    /** The time a request may take, in seconds: the tool is started with it and this provider waits a little longer. Default 120 */
    void SetRequestTimeoutSecs( UInt32 seconds );

    /** How often a child process was started by this provider */
    UInt32 GetStartCount( void ) const;

    virtual bool IsAvailable( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetUnavailableReason( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DecodeBlock( const void* compressed     ,
                              UInt32 compressedSize      ,
                              UInt32 decompressedSize    ,
                              void* dest                 ,
                              CORE::CString& error       ,
                              Int32& toolCode            ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetCancellationSource( const ASSETCONVERTER::CConversionReport* report ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum EAttempt
    {
        ATTEMPT_DONE = 0    ,   /**< the block is decoded */
        ATTEMPT_FAILED      ,   /**< the request failed and a second try would not help */
        ATTEMPT_LOST            /**< the child is gone or does not follow the protocol: a second try may help */
    };

    COodleToolProvider( const COodleToolProvider& src );            /**< not implemented */
    COodleToolProvider& operator=( const COodleToolProvider& src ); /**< not implemented */

    bool StartChild( CORE::CString& error );
    void StopChild( void );
    EAttempt TryBlock( const void* compressed, UInt32 compressedSize, UInt32 decompressedSize, void* dest, CORE::CString& error, Int32& toolCode );
    bool ReadAnswer( CORE::CString& line, CORE::CString& error, bool& cancelled );
    bool IsCancelled( void ) const;

    private:

    CORE::CString m_toolPath;
    CORE::CString m_workFolder;
    CORE::CProcess m_process;
    bool m_running;
    UInt32 m_requestTimeoutSecs;
    UInt32 m_startCount;
    UInt32 m_requestCounter;
    GUCEF::vector< char > m_answerBuffer;
    const ASSETCONVERTER::CConversionReport* m_cancellation;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H ? */
