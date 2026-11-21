/*
 *  AssetWorkbenchLib: a logger that collects the warnings and errors of the workbench for its report view
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

#ifndef ASSETWORKBENCHLIB_CREPORTLOGGER_H
#define ASSETWORKBENCHLIB_CREPORTLOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_GUI_CREPORTVIEWFORM_H
#include "gucefGUI_CReportViewForm.h"
#define GUCEF_GUI_CREPORTVIEWFORM_H
#endif /* GUCEF_GUI_CREPORTVIEWFORM_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The workbench's own log for its report view: what the workbench, Ogre and the plugins say as a warning or an error (load errors of
 *  a model, a texture that is not found) is kept by every thread that logs it and handed to the report form by Pump(), which the
 *  application calls on the GUI thread. Messages of a lower level than warnings are not kept. The log manager has to know the
 *  logger (CLogManager::AddLogger()) for as long as it lives.
 *
 *  Two kinds of message are not failures and are folded into one line of the report each, with the number and the first
 *  name: a file that is looked up and is not in the mounted pack (the VFS logs that as an error, but the caller falls back
 *  to the default texture or material: shown as a warning) and a file name that exists in more than one folder of the
 *  pack (Ogre warns about the second and later ones; a bare name finds the first folder, a name with a folder in it finds
 *  the right one: shown as information) and a mesh with vertices that have more than four bone influences (Ogre keeps the
 *  four strongest, renormalised, and says so for every mesh: shown as information, it is an approximation of the model
 *  and not a failure). The diagnostics of the model importer (Assimp calls its complaints about a property of a material
 *  errors, the import goes on and a failed import is reported by the model view itself) are shown as warnings. The line comes with the first Pump() after the messages stopped, so a long load
 *  gives one line and not one per frame. Everything else keeps the severity it has.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CReportLogger : public CORE::CILogger
{
    public:

    CReportLogger( void );

    virtual ~CReportLogger() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void SetMinimalLogLevel( const Int32 minimalLogLevel ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The kinds of message that are folded */
    enum EFold
    {
        FOLD_NONE = -1          ,
        FOLD_FILE_NOT_IN_PACK = 0,
        FOLD_NAME_IN_MORE_FOLDERS ,
        FOLD_WEIGHTS_REDUCED    ,
        FOLD_COUNT
    };

    /** Which kind of message this is: the text decides. The name of the file is given when there is one */
    static EFold ClassifyMessage( const CORE::CString& logMessage, CORE::CString& name );

    /** Whether this is a complaint of the model importer about its input that does not stop the import */
    static bool IsImporterDiagnostic( const CORE::CString& logMessage );

    /** The one line of a kind, for 'count' messages of which the first named 'first' */
    static CORE::CString MakeFoldedMessage( EFold fold, UInt32 count, const CORE::CString& first );

    /** Gives the entries that came since the last call to a report form (on the thread of the GUI); the number given. NULL drops them */
    UInt32 Pump( GUI::CReportViewForm* report );

    /** The number of entries kept for the next Pump() */
    UInt32 GetPendingCount( void ) const;

    /** How many warnings and errors this logger saw in all */
    UInt32 GetWarningCount( void ) const;
    UInt32 GetErrorCount( void ) const;

    private:

    CReportLogger( const CReportLogger& src );
    CReportLogger& operator=( const CReportLogger& src );

    void Keep( const CORE::TLogMsgType logMsgType, const CORE::CString& logMessage );

    private:

    mutable MT::CMutex m_lock;
    GUCEF::vector< GUI::CReportViewForm::TEntry > m_pending;
    UInt32 m_warnings;
    UInt32 m_errors;
    UInt32 m_foldCount[ FOLD_COUNT ];
    CORE::CString m_foldFirst[ FOLD_COUNT ];
    bool m_foldNewSincePump;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CREPORTLOGGER_H ? */
