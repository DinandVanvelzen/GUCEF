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

#ifndef GUCEF_DRN_CDRNFILERESOURCESINK_H
#define GUCEF_DRN_CDRNFILERESOURCESINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

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
 *  A file as the target of a transfer, resumable across reconnects and restarts.
 *
 *  The data goes into "<target>.drnpart" at its offsets (the file grows as needed, sparse where the file system
 *  allows), the state (CDRNResourcePartialState) into "<target>.drnstate", replaced atomically and durably
 *  (CORE::WriteFileAtomically) after the data was flushed, so the state never records a chunk that was not written.
 *  Finish() moves the part file over the target in one step (CORE::MoveFileAtomically) and removes the state. A
 *  transfer that ends otherwise keeps both for the next one, unless told to discard them.
 *
 *  Only a window that is the whole resource is resumable; a ranged window into a file starts empty.
 */
class GUCEF_DRN_EXPORT_CPP CDRNFileResourceSink : public CIDRNResourceSink
{
    public:

    explicit CDRNFileResourceSink( const CORE::CString& targetPath );

    virtual ~CDRNFileResourceSink() GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetTargetPath( void ) const;
    CORE::CString GetPartPath( void ) const;
    CORE::CString GetStatePath( void ) const;

    virtual bool PeekPartialState( CDRNResourcePartialState& state ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Begin( const CORE::CString& provider       ,
                                      const CDRNResourceInfo& info        ,
                                      UInt64 windowOffset                 ,
                                      UInt64 windowLength                 ,
                                      bool resume                         ,
                                      CDRNResourcePartialState& state     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Write( UInt64 offset, const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Commit( const CDRNResourcePartialState& state ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus ReadBack( UInt64 offset, UInt32 size, void* destination ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDRNResourceStatus Finish( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Abort( const CDRNResourcePartialState& state, bool discard ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsResumable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNFileResourceSink( const CDRNFileResourceSink& src );               /**< not implemented */
    CDRNFileResourceSink& operator=( const CDRNFileResourceSink& src );    /**< not implemented */

    bool SaveState( const CDRNResourcePartialState& state );
    void RemovePartial( void );

    CORE::CString m_targetPath;
    CORE::CFileAccess m_file;
    UInt64 m_windowOffset;
    UInt64 m_windowLength;
    bool m_wholeResource;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNFILERESOURCESINK_H ? */
