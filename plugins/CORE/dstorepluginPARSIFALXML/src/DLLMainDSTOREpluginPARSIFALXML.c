/*
 *  dstorepluginPARSIFALXML: Plugin adding the ability to use Parsifal XML
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>     /* memory management */
#include <stdio.h>      /* standard I/O utils */
#include <string.h>     /* standard string utils */

#ifndef PARSIFAL__H
#include "parsifal.h"   /* Parsifal XML Parser API */
#define PARSIFAL__H
#endif /* PARSIFAL__H ? */

/* Parsifal defines BYTE in the global namesapce which can cause problems :( */
#undef BYTE

#include "DLLMainDSTOREpluginPARSIFALXML.h"    /* gucefCORE DSTORE codec plugin API */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* gucefCORE macros, used here for the export and callspec macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define NODE_STORE_INDENT       1
#define INDENT_CHAR             9  /* horizontal tab */

#define VERSION_MAJOR_FIELD     1
#define VERSION_MINOR_FIELD     1
#define VERSION_PATCH_FIELD     0
#define VERSION_RELEASE_FIELD   0

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define strdup _strdup
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SDestFileData
{
    TIOAccess* fptr;
    Int32 indent;
    char* line;
    UInt32 linelen;

    char* buffer;
    UInt32 bufferSize;

    Int32 currentNodeType;
};

typedef struct SDestFileData TDestFileData;

/*---------------------------------------------------------------------------*/

struct SSrcFileData
{
    TIOAccess* access;
    LPXMLPARSER parser;
    TReadHandlers handlers;
    void* privdata;
    XMLSTRINGBUF textBuffer;
};

typedef struct SSrcFileData TSrcFileData;

/*---------------------------------------------------------------------------*/

struct SEscapeEntry
{
    char escapeChar;
    const char* replacementStr;
    char replacementStrLen;
};

typedef struct SEscapeEntry TEscapeEntry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TGucefCoreCApi g_libApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
ParsifalReader( unsigned char* buf ,
                int cbytes         ,
                int *cbytesactual  ,
                void* inputdata    )
{
        *cbytesactual = (int) ((TIOAccess*)inputdata)->read( ((TIOAccess*)inputdata), buf, 1, cbytes );
        return (*cbytesactual < cbytes);
}

/*---------------------------------------------------------------------------*/

static void
ParsifalError( struct tagXMLPARSER *parser )
{
        TSrcFileData* sd = (TSrcFileData*) parser->UserData;
        (*sd->handlers.OnError)( sd->privdata, parser->ErrorCode, parser->ErrorString );
}

/*---------------------------------------------------------------------------*/

static int
ParsifalDocumentStart( void* userdata )
{
        TSrcFileData* sd = (TSrcFileData*) userdata;
        (*sd->handlers.OnTreeBegin)( sd->privdata );
        return XML_OK;
}

/*---------------------------------------------------------------------------*/

static int
ParsifalDocumentEnd( void* userdata )
{
        TSrcFileData* sd = (TSrcFileData*) userdata;
        (*sd->handlers.OnTreeEnd)( sd->privdata );
        return XML_OK;
}

/*---------------------------------------------------------------------------*/

static int
ParsifalCharacters( void *userdata     ,
                    const XMLCH *chars ,
                    int cbChars        )
{
    TSrcFileData* sd = (TSrcFileData*) userdata;
	XMLStringbuf_Append( &sd->textBuffer, (XMLCH*)chars, cbChars );
	return XML_OK;
}
/*---------------------------------------------------------------------------*/

static int
ParsifalElementStart( void* userdata         ,
                      const XMLCH *uri       ,
                      const XMLCH *localname ,
                      const XMLCH *qname     ,
                      LPXMLVECTOR atts       )
{
    Int32 i;
    LPXMLRUNTIMEATT att;
    TVariantData key;
    TVariantData var;

    TSrcFileData* sd = (TSrcFileData*) userdata;

    (*sd->handlers.OnNodeBegin)( sd->privdata, qname, GUCEF_DATATYPE_OBJECT );
    if ( atts->length )
    {
        for ( i=0; i<atts->length; ++i )
        {            
            att = (LPXMLRUNTIMEATT) XMLVector_Get( atts, i );
            
            memset( &key, 0, sizeof( key ) );
            key.containedType = GUCEF_DATATYPE_UTF8_STRING; 
            key.union_data.heap_data.heap_data_is_linked = 1;
            key.union_data.heap_data.heap_data_size = (UInt32) strlen( att->qname );
            key.union_data.heap_data.union_data.char_heap_data = att->qname;

            memset( &var, 0, sizeof( var ) );
            var.containedType = GUCEF_DATATYPE_UTF8_STRING; 
            var.union_data.heap_data.heap_data_is_linked = 1;
            var.union_data.heap_data.heap_data_size = (UInt32) strlen( att->value );
            var.union_data.heap_data.union_data.char_heap_data = att->value;
            
            (*sd->handlers.OnNodeAtt)( sd->privdata ,
                                       qname        ,
                                       &key         ,
                                       &var         );
        }
    }
    return XML_OK;
}

/*---------------------------------------------------------------------------*/

void
GuaranteeMinBufferSize( char** destBuffer        ,
                        UInt32* destBufferLength ,
                        UInt32 desiredSize       )
{
    if ( NULL != destBuffer && NULL != destBufferLength )
    {
        if ( NULL != *destBuffer )
        {
            if ( desiredSize > *destBufferLength )
            {
                *destBuffer = realloc( *destBuffer, desiredSize );
                *destBufferLength = desiredSize;
            }
        }
        else
        {
            *destBuffer = malloc( desiredSize );
            *destBufferLength = desiredSize;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
HandleEscapeCharacterSet( const char* srcStr         ,
                          UInt32 srcStrLen           ,
                          char** destBuffer          ,
                          UInt32* destBufferLength   ,
                          TEscapeEntry* escapeEntry  ,
                          UInt32 escapeEntryCount    ,
                          UInt32* strLen             )
{
    UInt32 n = 0;
    UInt32 i = 0;
    UInt32 replCount = 0;
    UInt32 replSize = 0;
    char* bufferPtr = NULL;

    for ( i=0; i<srcStrLen; ++i )
    {
        for ( n=0; n<escapeEntryCount; ++n )
        {
            if ( srcStr[ i ] == escapeEntry[ n ].escapeChar )
            {
                replCount++;
                replSize += escapeEntry[ n ].replacementStrLen;
                break;
            }
        }
    }

    *strLen = srcStrLen-replCount+replSize;
    GuaranteeMinBufferSize( destBuffer, destBufferLength, (*strLen)+1 );
    bufferPtr = *destBuffer;
    memset( bufferPtr, 0, (*strLen)+1 );

    for ( i=0; i<srcStrLen; ++i )
    {
        char foundEscChar = 0;
        for ( n=0; n<escapeEntryCount; ++n )
        {
            if ( srcStr[ i ] == escapeEntry[ n ].escapeChar )
            {
                foundEscChar = 1;
                memcpy( bufferPtr, escapeEntry[ n ].replacementStr, escapeEntry[ n ].replacementStrLen );
                bufferPtr += escapeEntry[ n ].replacementStrLen;
                break;
            }
        }

        if ( 0 == foundEscChar )
        {
            *bufferPtr = srcStr[ i ];
            ++bufferPtr;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
PrintNumericVariant( const TVariantData* src  ,
                     char* destBuffer         ,
                     UInt32 destBufferLength  ,
                     UInt32* strLen           ,
                     Int32* wasNumeric        )
{
    if ( GUCEF_NULL != src )
    {
        switch ( src->containedType )
        {
            /* at init we already make sue the line buffer is large enough for numeric values
             * as such no need to worry about that here
             */
            case GUCEF_DATATYPE_INT8:    { snprintf( destBuffer, destBufferLength, "%d", (Int32) src->union_data.int8_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT8:   { snprintf( destBuffer, destBufferLength, "%u", (UInt32) src->union_data.uint8_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_INT16:   { snprintf( destBuffer, destBufferLength, "%d", (Int32) src->union_data.int16_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT16:  { snprintf( destBuffer, destBufferLength, "%u", (UInt32) src->union_data.uint16_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_INT32:   { snprintf( destBuffer, destBufferLength, "%d", src->union_data.int32_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT32:  { snprintf( destBuffer, destBufferLength, "%u", src->union_data.uint32_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            #ifdef GUCEF_MSWIN_BUILD
            case GUCEF_DATATYPE_INT64:   { snprintf( destBuffer, destBufferLength, "%I64d", src->union_data.int64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT64:  { snprintf( destBuffer, destBufferLength, "%I64u", src->union_data.uint64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            #else
            case GUCEF_DATATYPE_INT64:   { snprintf( destBuffer, destBufferLength, "%lld", src->union_data.int64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT64:  { snprintf( destBuffer, destBufferLength, "%llu", src->union_data.uint64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            #endif
            case GUCEF_DATATYPE_FLOAT32: { snprintf( destBuffer, destBufferLength, "%f", src->union_data.float32_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_FLOAT64: { snprintf( destBuffer, destBufferLength, "%lf", src->union_data.float64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
        
            default:
            {
                *destBuffer = '\0';
                *strLen = 0;
                *wasNumeric = 0;
                break;
            }
        }
    }
    else
    {
        *destBuffer = '\0';
        *strLen = 0;
        *wasNumeric = 0;
    }
}

/*---------------------------------------------------------------------------*/

void
HandleEscapeCharacters( const char* srcStr       ,
                        char** destBuffer        ,
                        UInt32* destBufferLength ,
                        UInt32* strLen           )
{
    if ( GUCEF_NULL != srcStr )
    {
        TEscapeEntry entries[ 6 ];
        UInt32 srcStrLen = (UInt32) strlen( srcStr );

        entries[ 0 ].escapeChar = '<';
        entries[ 0 ].replacementStr = "&lt;";
        entries[ 0 ].replacementStrLen = 4;
        entries[ 1 ].escapeChar = '>';
        entries[ 1 ].replacementStr = "&gt;";
        entries[ 1 ].replacementStrLen = 4;
        entries[ 2 ].escapeChar = '&';
        entries[ 2 ].replacementStr = "&amp;";
        entries[ 2 ].replacementStrLen = 5;
        entries[ 3 ].escapeChar = '\'';
        entries[ 3 ].replacementStr = "&apos;";
        entries[ 3 ].replacementStrLen = 6;
        entries[ 4 ].escapeChar = '\"';
        entries[ 4 ].replacementStr = "&quot;";
        entries[ 4 ].replacementStrLen = 6;
        entries[ 5 ].escapeChar = '»';
        entries[ 5 ].replacementStr = "&#187;";
        entries[ 5 ].replacementStrLen = 6;

        HandleEscapeCharacterSet( srcStr           ,
                                  srcStrLen        ,
                                  destBuffer       ,
                                  destBufferLength ,
                                  entries          ,
                                  6                ,
                                  strLen           );
        return;
    }

    /* default: replace with empty string */
    *strLen = 0;
    if ( *destBufferLength > 0 )
    {
        memset( *destBuffer, 0, *destBufferLength );
    }
}

/*---------------------------------------------------------------------------*/

void
HandleEscapeCharactersInVariant( const TVariantData* src  ,
                                 char** destBuffer        ,
                                 UInt32* destBufferLength ,
                                 UInt32* strLen           )
{
    if ( GUCEF_NULL != src )
    {
        Int32 isNumeric = 0;
        PrintNumericVariant( src, *destBuffer, *destBufferLength, strLen, &isNumeric );

        if ( 0 == isNumeric )
        {
            *strLen = src->union_data.heap_data.heap_data_size;
            const char* srcStr = src->union_data.heap_data.union_data.char_heap_data;
            HandleEscapeCharacters( srcStr, destBuffer, destBufferLength, strLen );
            return;
        }
    }

    /* default: replace with empty string */
    *strLen = 0;
    if ( *destBufferLength > 0 )
    {
        memset( *destBuffer, 0, *destBufferLength );
    }
}

/*---------------------------------------------------------------------------*/

static int
ParsifalElementEnd( void* userdata         ,
                    const XMLCH *uri       ,
                    const XMLCH *localname ,
                    const XMLCH *qname     )
{
    TSrcFileData* sd = (TSrcFileData*) userdata;

    char* text = strdup( (const char*) XMLStringbuf_ToString( &sd->textBuffer ) );
    if ( 0 != text )
    {
        /* normalize buffer, note that XMLNormalizeBuf doesn't nul terminate the buffer: */
		int len = XMLNormalizeBuf( text, sd->textBuffer.len );
		if ( len < sd->textBuffer.len ) 
            text[ len ] = '\0';

        if ( len > 0 )
        {
            TVariantData var;
            memset( &var, 0, sizeof( var ) );
            var.containedType = GUCEF_DATATYPE_UTF8_STRING; 
            var.union_data.heap_data.heap_data_is_linked = 1;
            var.union_data.heap_data.heap_data_size = len;
            var.union_data.heap_data.union_data.char_heap_data = text;

            (*sd->handlers.OnNodeValue)( sd->privdata ,
                                         qname        ,
                                         &var         );
        }

        /* we'll reuse Stringbuf just setting its length to 0: */
		XMLStringbuf_SetLength( &sd->textBuffer, 0 );
    }

    (*sd->handlers.OnNodeEnd)( sd->privdata, qname );
    return XML_OK;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Init( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* parsifal itself does not require global initialization */
    if ( GUCEF_NULL != plugdata )
        *plugdata = GUCEF_NULL;

    /* link the gucefCORE API */
    if ( GUCEF_NULL != libApi )
        g_libApi = *libApi;

    return 1;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Shutdown( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* parsifal itself does not require global cleanup */

    memset( &g_libApi, 0, sizeof g_libApi );
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Open( void** plugdata    ,
                           void** filedata    ,
                           TIOAccess* outFile ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL == filedata )
        return 0;    
    *filedata = GUCEF_NULL;

    if ( outFile != GUCEF_NULL )
    {
        char outBuffer[ 275 ];
        TDestFileData* fd = (TDestFileData*) malloc( sizeof( TDestFileData ) );
        if ( GUCEF_NULL == fd )
            return 0;
        fd->fptr = outFile;
        fd->indent = 0;
        fd->line = GUCEF_NULL;
        fd->linelen = 0;
        fd->buffer = GUCEF_NULL;
        fd->bufferSize = 0;
        fd->currentNodeType = GUCEF_DATATYPE_UNKNOWN;
        *filedata = fd;

        /* start with a buffer large enough to hold 64 bit numbers when printed */
        GuaranteeMinBufferSize( &fd->line, &fd->linelen, 21 );
                
        sprintf( outBuffer, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\r\n" );
        outFile->write( outFile, outBuffer, (UInt32)strlen( outBuffer ), 1 );
        sprintf( outBuffer, "<!-- Generated using dstorepluginXMLPARSIFAL (v%d.%d.%d.%d) for the GUCEF platform - Copyright (C) Dinand Vanvelzen. LPGLv3 -->\r\n", VERSION_MAJOR_FIELD, VERSION_MINOR_FIELD, VERSION_PATCH_FIELD, VERSION_RELEASE_FIELD );
        outFile->write( outFile, outBuffer, (UInt32)strlen( outBuffer ), 1 );
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Close( void** plugdata ,
                            void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( filedata )
    {
        if ( *filedata )
        {
            TDestFileData* fd = (TDestFileData*)*filedata;
            fd->fptr->close( fd->fptr );
            if ( fd->line && fd->linelen )
            {
                free( fd->line );
                fd->line = NULL;
            }
            if ( fd->buffer && fd->bufferSize )
            {
                free( fd->buffer );
                fd->buffer = NULL;
            }
            free( fd );
        }
    }
}

/*---------------------------------------------------------------------------*/

const char* 
NodeNameFromType( Int32 type )
{
    switch ( type )
    {
        case GUCEF_DATATYPE_UINT8:
            return GUCEF_DATATYPE_NAME_UINT8;
        case GUCEF_DATATYPE_INT8:
            return GUCEF_DATATYPE_NAME_INT8;
        case GUCEF_DATATYPE_LE_UINT16:
            return GUCEF_DATATYPE_NAME_LE_UINT16;
        case GUCEF_DATATYPE_BE_UINT16:
            return GUCEF_DATATYPE_NAME_BE_UINT16;
        case GUCEF_DATATYPE_LE_INT16:
            return GUCEF_DATATYPE_NAME_LE_INT16;
        case GUCEF_DATATYPE_BE_INT16:
            return GUCEF_DATATYPE_NAME_BE_INT16;
        case GUCEF_DATATYPE_LE_UINT32:
            return GUCEF_DATATYPE_NAME_LE_UINT32;
        case GUCEF_DATATYPE_BE_UINT32:
            return GUCEF_DATATYPE_NAME_BE_UINT32;
        case GUCEF_DATATYPE_LE_INT32:
            return GUCEF_DATATYPE_NAME_LE_INT32;
        case GUCEF_DATATYPE_BE_INT32:
            return GUCEF_DATATYPE_NAME_BE_INT32;
        case GUCEF_DATATYPE_LE_UINT64:
            return GUCEF_DATATYPE_NAME_LE_UINT64;
        case GUCEF_DATATYPE_BE_UINT64:
            return GUCEF_DATATYPE_NAME_BE_UINT64;
        case GUCEF_DATATYPE_LE_INT64:
            return GUCEF_DATATYPE_NAME_LE_INT64;
        case GUCEF_DATATYPE_BE_INT64:
            return GUCEF_DATATYPE_NAME_BE_INT64;
        case GUCEF_DATATYPE_LE_FLOAT32:
            return GUCEF_DATATYPE_NAME_LE_FLOAT32;
        case GUCEF_DATATYPE_BE_FLOAT32:
            return GUCEF_DATATYPE_NAME_BE_FLOAT32;
        case GUCEF_DATATYPE_LE_FLOAT64:
            return GUCEF_DATATYPE_NAME_LE_FLOAT64;
        case GUCEF_DATATYPE_BE_FLOAT64:
            return GUCEF_DATATYPE_NAME_BE_FLOAT64;

        case GUCEF_DATATYPE_NUMERIC:
            return GUCEF_DATATYPE_NAME_NUMERIC; 
        case GUCEF_DATATYPE_ASCII_STRING:
            return GUCEF_DATATYPE_NAME_ASCII_STRING;
        case GUCEF_DATATYPE_UTF8_STRING:
            return GUCEF_DATATYPE_NAME_UTF8_STRING;
        case GUCEF_DATATYPE_UTF16_LE_STRING:
            return GUCEF_DATATYPE_NAME_UTF16_LE_STRING;
        case GUCEF_DATATYPE_UTF16_BE_STRING:
            return GUCEF_DATATYPE_NAME_UTF16_BE_STRING;
        case GUCEF_DATATYPE_UTF32_LE_STRING:
            return GUCEF_DATATYPE_NAME_UTF32_LE_STRING;
        case GUCEF_DATATYPE_UTF32_BE_STRING:
            return GUCEF_DATATYPE_NAME_UTF32_BE_STRING;

        case GUCEF_DATATYPE_BOOLEAN_INT32:
            return GUCEF_DATATYPE_NAME_BOOLEAN_INT32;
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
            return GUCEF_DATATYPE_NAME_BOOLEAN_ASCII_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
            return GUCEF_DATATYPE_NAME_BOOLEAN_UTF8_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF16_LE_STRING:
            return GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_LE_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF16_BE_STRING:
            return GUCEF_DATATYPE_NAME_BOOLEAN_UTF16_BE_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF32_LE_STRING:
            return GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_LE_STRING;
        case GUCEF_DATATYPE_BOOLEAN_UTF32_BE_STRING:
            return GUCEF_DATATYPE_NAME_BOOLEAN_UTF32_BE_STRING;

        case GUCEF_DATATYPE_BINARY_BLOB:
            return GUCEF_DATATYPE_NAME_BINARY_BLOB;
        case GUCEF_DATATYPE_BINARY_BSOB:
            return GUCEF_DATATYPE_NAME_BINARY_BSOB;

        case GUCEF_DATATYPE_DATETIME_ISO8601_ASCII_STRING:
            return GUCEF_DATATYPE_NAME_DATETIME_ISO8601_ASCII_STRING;
        case GUCEF_DATATYPE_DATETIME_ISO8601_UTF8_STRING:
            return GUCEF_DATATYPE_NAME_DATETIME_ISO8601_UTF8_STRING;

        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            return GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
            return GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH;
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
            return GUCEF_DATATYPE_NAME_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
            return GUCEF_DATATYPE_NAME_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;

        case GUCEF_DATATYPE_NIL: 
            return GUCEF_DATATYPE_NAME_NIL;
        case GUCEF_DATATYPE_NULL: 
            return GUCEF_DATATYPE_NAME_NULL;
        case GUCEF_DATATYPE_SET: 
            return GUCEF_DATATYPE_NAME_SET;
        case GUCEF_DATATYPE_ARRAY: 
            return GUCEF_DATATYPE_NAME_ARRAY;

        case GUCEF_DATATYPE_OBJECT:
        default:
            return "noname";
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Store( void** plugdata      ,
                             void** filedata      ,
                             const char* nodename ,
                             Int32 nodeType       ,
                             UInt32 attscount     ,
                             UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*)*filedata;
    UInt32 max, len, strLen=0;
    char* escNodeName = GUCEF_NULL;

    if ( GUCEF_NULL == fd ) 
        return;
    fd->currentNodeType = nodeType;

    if ( GUCEF_NULL == nodename )
        nodename = NodeNameFromType( fd->currentNodeType );         
    if ( *nodename == '\0' )
        nodename = NodeNameFromType( fd->currentNodeType );

    HandleEscapeCharacters( nodename, &fd->buffer, &fd->bufferSize, &strLen );
    escNodeName = (char*) malloc( strLen+1 );
    if ( GUCEF_NULL == escNodeName )
        return;
    memcpy( escNodeName, fd->buffer, strLen+1 );

    len = (UInt32)strlen( escNodeName );
    max = len + 5*attscount + 6 + fd->indent;
    if ( fd->linelen < max )
    {
        char* newLine = realloc( fd->line, max );
        if ( GUCEF_NULL != newLine )
        {
            fd->line = newLine;
            *(fd->line) = 0;
            fd->linelen = max;
        }
    }
    memset( fd->line, INDENT_CHAR, fd->indent );
    if ( attscount > 0 )
    {
        sprintf( fd->line+fd->indent, "<%s ", escNodeName );
    }
    else
    {
        haschildren ? sprintf( fd->line+fd->indent, "<%s>\r\n", escNodeName ) : sprintf( fd->line+fd->indent, "<%s/>\r\n", escNodeName );
        fd->fptr->write( fd->fptr, fd->line, (UInt32)strlen( fd->line ), 1 );
    }

    free( escNodeName );
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Store( void** plugdata      ,
                           void** filedata      ,
                           const char* nodename ,
                           UInt32 attscount     ,
                           UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*) *filedata;

    if ( GUCEF_NULL == fd ) 
        return;
    if ( GUCEF_NULL == nodename )
        nodename = NodeNameFromType( fd->currentNodeType );         
    if ( *nodename == '\0' )
        nodename = NodeNameFromType( fd->currentNodeType );

    if ( haschildren )
    {        
        UInt32 strLen = 0;
        char* escNodeName = NULL;

        HandleEscapeCharacters( nodename, &fd->buffer, &fd->bufferSize, &strLen );
        escNodeName = (char*) malloc( strLen+1 );
        if ( GUCEF_NULL == escNodeName )
            return;
        memcpy( escNodeName, fd->buffer, strLen+1 );

        memset( fd->line, INDENT_CHAR, fd->indent );
        sprintf( fd->line+fd->indent, "</%s>\r\n", escNodeName );
        fd->fptr->write( fd->fptr, fd->line, (UInt32)strlen( fd->line ), 1 );

        free( escNodeName );
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Store_Node_Att( void** plugdata              ,
                           void** filedata              ,
                           const char* nodename         ,
                           UInt32 attscount             ,
                           UInt32 attindex              ,
                           const char* attname          ,
                           const TVariantData* attvalue ,
                           UInt32 haschildren           ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*)*filedata;
    UInt32 linelen = (UInt32)strlen( fd->line ); 
    UInt32 len = 0;
    UInt32 strLen = 0;
    char* escAttName = GUCEF_NULL;
    char* escAttValue = GUCEF_NULL;

    if ( GUCEF_NULL == fd ) 
        return;
    if ( GUCEF_NULL == nodename )
        nodename = NodeNameFromType( fd->currentNodeType );         
    if ( *nodename == '\0' )
        nodename = NodeNameFromType( fd->currentNodeType );

    if ( 0 == attscount ) 
        return;

    if ( GUCEF_NULL == attvalue )
    {
        static TVariantData dummy;
        memset( &dummy, 0, sizeof dummy );
        attvalue = &dummy;
    }

    HandleEscapeCharactersInVariant( attvalue, &fd->buffer, &fd->bufferSize, &strLen );
    escAttValue = (char*) malloc( strLen+1 );
    if ( GUCEF_NULL == escAttValue )
        return;
    memcpy( escAttValue, fd->buffer, strLen+1 );

    if ( GUCEF_NULL != attname )
    {
        HandleEscapeCharacters( attname, &fd->buffer, &fd->bufferSize, &strLen );
        escAttName = (char*) malloc( strLen+1 );
        if ( GUCEF_NULL == escAttName )
        {
            free( escAttValue );
            return;
        }
        memcpy( escAttName, fd->buffer, strLen+1 );

        len = (UInt32)strlen( escAttName )+(UInt32)strlen( escAttValue )+2;
        if ( fd->linelen < linelen+10+len )
        {
            GuaranteeMinBufferSize( &fd->line, &fd->linelen, linelen+11+len ); 
        }
        if ( attindex+1 < attscount )
        {
            sprintf( fd->line+linelen, "%s=\"%s\" ", escAttName, escAttValue );
            return;
        }
        if ( haschildren )
        {
            sprintf( fd->line+linelen, "%s=\"%s\" >\r\n", escAttName, escAttValue );
        }
        else
        {
            sprintf( fd->line+linelen, "%s=\"%s\" />\r\n", escAttName, escAttValue );
        }
        fd->fptr->write( fd->fptr, fd->line, (UInt32)strlen( fd->line ), 1 );

        free( escAttName );
        
    }
    else
    {
        len = (UInt32)strlen( escAttValue )+(UInt32)strlen( nodename );
        if ( fd->linelen < linelen+10+len )
        {
            GuaranteeMinBufferSize( &fd->line, &fd->linelen, linelen+11+len );
        }
        sprintf( fd->line+linelen-1, ">%s</%s>\r\n", escAttValue, nodename );
        fd->fptr->write( fd->fptr, fd->line, (UInt32)strlen( fd->line ), 1 );
    }

    free( escAttValue );
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Children( void** plugdata      ,
                                void** filedata      ,
                                const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*)*filedata;
    fd->indent += NODE_STORE_INDENT;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Children( void** plugdata      ,
                              void** filedata      ,
                              const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*)*filedata;
    fd->indent -= NODE_STORE_INDENT;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Open( void** plugdata  ,
                          void** codecdata ,
                          void** filedata  ,
                          TIOAccess* file  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSrcFileData* sd;
    *plugdata = NULL;

    sd = (TSrcFileData*) malloc( sizeof(TSrcFileData) );
    if ( GUCEF_NULL == sd )
        return 0;
    sd->access = file;

    if ( XMLParser_Create( &sd->parser ) )
    {
        sd->privdata = NULL;
        memset( &sd->handlers, 0, sizeof(TReadHandlers) );

        /* init Stringbuf: blockSize 256, no pre-allocation: */
        XMLStringbuf_Init( &sd->textBuffer, 256, 0 );

        sd->parser->startDocumentHandler = &ParsifalDocumentStart;
        sd->parser->endDocumentHandler = &ParsifalDocumentEnd;
        sd->parser->startElementHandler = &ParsifalElementStart;
        sd->parser->endElementHandler = &ParsifalElementEnd;
        sd->parser->charactersHandler = &ParsifalCharacters;
        sd->parser->errorHandler = &ParsifalError;
        sd->parser->UserData = sd;
        *filedata = sd;
        return 1;
    }
    free( sd );
    return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Close( void** plugdata  ,
                           void** codecdata ,
                           void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        if ( *filedata )
        {
                TSrcFileData* sd = (TSrcFileData*) *filedata;
                XMLParser_Free( sd->parser );
                sd->access->close( sd->access );
        }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Set_Read_Handlers( void** plugdata                ,
                              void** codecdata               ,
                              void** filedata                ,
                              const TReadHandlers* rhandlers ,
                              void* privdata                 ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        if ( *filedata )
        {
                TSrcFileData* sd = (TSrcFileData*) *filedata;
                sd->handlers = *rhandlers;
                sd->privdata = privdata;
        }
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Start_Reading( void** plugdata  ,
                          void** codecdata ,
                          void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL != filedata )
    {
        TSrcFileData* sd = (TSrcFileData*) *filedata;
        if ( GUCEF_NULL != sd )
        {
            int result = XMLParser_Parse( sd->parser      ,
                                          &ParsifalReader ,
                                          sd->access      ,
                                          NULL            );
            if ( XML_OK == result )
                return 0; /* no errorcode == ok for our API */

            return (UInt32) sd->parser->ErrorCode;
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Type( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "xml";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Name( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        static char buffer[ 164 ];
        sprintf( buffer, "gucefCORE DSTORE codec plugin with XML writing code and utilizing the Parsifal XML parser %s for reading", XMLParser_GetVersionString() );
        return buffer;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Copyright( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        return "Copyright (C) Dinand Vanvelzen. 2002 - 2005. LGPLv3.";
}

/*---------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Version( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        static TVersion version;
        version.major = VERSION_MAJOR_FIELD;
        version.minor = VERSION_MINOR_FIELD;
        version.patch = VERSION_PATCH_FIELD;
        version.release = VERSION_RELEASE_FIELD;

        return &version;
}

/*---------------------------------------------------------------------------*/
