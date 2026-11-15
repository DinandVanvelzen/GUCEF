/*
 *  AssetViewerLib: a logger that collects the warnings and errors of the viewer for its report view
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

#ifndef ASSETVIEWERLIB_CREPORTLOGGER_H
#define ASSETVIEWERLIB_CREPORTLOGGER_H

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

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The viewer's own log for its report view: what the viewer, Ogre and the plugins say as a warning or an error (load errors of
 *  a model, a texture that is not found) is kept by every thread that logs it and handed to the report form by Pump(), which the
 *  application calls on the GUI thread. Messages of a lower level than warnings are not kept. The log manager has to know the
 *  logger (CLogManager::AddLogger()) for as long as it lives.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CReportLogger : public CORE::CILogger
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
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CREPORTLOGGER_H ? */
