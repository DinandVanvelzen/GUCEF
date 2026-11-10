/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once

#ifdef _MSC_VER
    //disable windows complaining about max template size.
    #pragma warning (disable : 4503)
#endif // _MSC_VER

#if defined (USE_WINDOWS_DLL_SEMANTICS) || defined (_WIN32)
    #ifdef _MSC_VER
        #pragma warning(disable : 4251)
    #endif // _MSC_VER

    #ifdef USE_IMPORT_EXPORT
        #ifdef AWS_S3_EXPORTS
            #define AWS_S3_API __declspec(dllexport)
        #else
            #define AWS_S3_API __declspec(dllimport)
        #endif /* AWS_S3_EXPORTS */
    #else
        #define AWS_S3_API
    #endif // USE_IMPORT_EXPORT
#else // defined (USE_WINDOWS_DLL_SEMANTICS) || defined (WIN32)
    // DV edit: upstream builds with default symbol visibility. Our builds hide symbols by default and the classes of the
    // library only have the API macro on their members, which leaves their vtables and typeinfo hidden, so keep the default
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC visibility push(default)
    #endif
    // DV edit: our builds hide symbols by default (-fvisibility=hidden) and upstream expects the default visibility on
    // non-Windows platforms, so the API macro has to export explicitly or the library cannot be used from other modules
    #if __GNUC__ >= 4
        #define AWS_S3_API __attribute__((visibility("default")))
    #else
        #define AWS_S3_API
    #endif
#endif // defined (USE_WINDOWS_DLL_SEMANTICS) || defined (WIN32)
