/*
 *  dstorepluginJSONPARSER: Plugin adding the ability to use jsonparser & jsonbuilder
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

#ifndef _JSON_H
#include "json.h"   /* json-parser JSON parser library in ANSI C */
#define _JSON_H
#endif /* _JSON_H ? */

#ifndef _JSON_BUILDER_H
#include "json-builder.h"  /* json-builder JSON serialization library in ANSI C */
#define _JSON_BUILDER_H
#endif /* _JSON_BUILDER_H ? */

#include "DLLMainDSTOREpluginJSONPARSER.h"    /* gucefCORE DSTORE codec plugin API */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* gucefCORE macros, used here for the export and callspec macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VERSION_MAJOR_FIELD     0
#define VERSION_MINOR_FIELD     1
#define VERSION_PATCH_FIELD     0
#define VERSION_RELEASE_FIELD   0

static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int base64_table[ 256 ] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
    0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SDestFileData
{
    TIOAccess* fptr;
    json_value* jsonDoc;
    json_value* currentJsonNode;
    char activeNodeIsValueNode;
    char* base64EncodeBuffer;
    UInt32 base64EncodeBufferSize;
};
typedef struct SDestFileData TDestFileData;

/*---------------------------------------------------------------------------*/

struct SSrcFileData
{
    TIOAccess* access;
    json_value* jsonDoc;
    void* privdata;
    TReadHandlers handlers;
};
typedef struct SSrcFileData TSrcFileData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( !defined stricmp )
  #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #define stricmp   _stricmp
  #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    #define stricmp   strcasecmp
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

char*
GetBase64EncodeBuffer( TDestFileData* fd, UInt32 spaceNeeded )
{    
    if ( fd->base64EncodeBufferSize < spaceNeeded )
    {
        char* newBuffer = (char*) realloc( fd->base64EncodeBuffer, spaceNeeded );
        if ( NULL != newBuffer )
        {
            fd->base64EncodeBuffer = newBuffer;
            fd->base64EncodeBufferSize = spaceNeeded;
        }
        return newBuffer;
    }
    return fd->base64EncodeBuffer;
}

/*-------------------------------------------------------------------------*/

int
Base64Encode( const void* inByteBuffer , 
              UInt32 inBufferSize      ,
              TDestFileData* fd        )
{GUCEF_TRACE;

    UInt32 base64StrLength = ( ( inBufferSize + 2 ) / 3 ) * 4;
    char* str = GetBase64EncodeBuffer( fd, base64StrLength+1 );         
    char* p = (char*) inByteBuffer;
    UInt32 j = 0, pad = inBufferSize % 3;
    const UInt32 last = inBufferSize - pad;

    // Base64 encoding can really add up wrt size requirements
    // We need to sanity check that we were able to allocate enough memory
    if ( GUCEF_NULL == str )
        return 0;

    memset( str, '=', base64StrLength );    

    for ( size_t i = 0; i < last; i+=3 )
    {
        int pi = (int) p[ i ];
        int piNext = (int) p[ i +1 ];
        int n = pi << 16 | piNext << 8 | p[ i + 2 ];
        str[ j++ ] = base64_chars[ n >> 18 ];
        str[ j++ ] = base64_chars[ n >> 12 & 0x3F ];
        str[ j++ ] = base64_chars[ n >> 6 & 0x3F ];
        str[ j++ ] = base64_chars[ n & 0x3F ];
    }

    if ( pad > 0 )
    {
        int pLast = (int) p[ last ];
        int n = --pad ? pLast << 8 | p[ last + 1 ] : p[ last ];
        str[ j++ ] = base64_chars[ pad ? n >> 10 & 0x3F : n >> 2 ];
        str[ j++ ] = base64_chars[ pad ? n >> 4 & 0x03F : n << 4 & 0x3F ];
        str[ j++ ] = pad ? base64_chars[ n << 2 & 0x3F ] : '=';
    }

    str[ base64StrLength ] = '\0';
    return 1;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Init( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* json-parser does not require global initialization */
    *plugdata = NULL;
    return 1;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Shutdown( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* json-parser does not require global cleanup */
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Open( void** plugdata    ,
                           void** filedata    ,
                           TIOAccess* outFile ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *filedata = GUCEF_NULL;
    if ( outFile != GUCEF_NULL )
    {
        TDestFileData* fd = (TDestFileData*) malloc( sizeof( TDestFileData ) );
        if ( NULL != fd )
        {
            memset( fd, 0, sizeof( TDestFileData ) );
            fd->fptr = outFile;
            fd->activeNodeIsValueNode = 0;
            fd->base64EncodeBuffer = NULL;
            fd->base64EncodeBufferSize = 0;
            *filedata = fd;
            return 1;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Close( void** plugdata ,
                            void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( filedata == GUCEF_NULL )
        return;
    if ( *filedata == GUCEF_NULL )
        return;

    TDestFileData* fd = (TDestFileData*)*filedata;
    if ( fd->jsonDoc != GUCEF_NULL )
    {
        /*
         * Thus far we have only created the DOM in json_value format
         * This is where we actually do the serialization from the DOM
         * we constructed
         */

        size_t measured = json_measure( fd->jsonDoc );
        char* serializationBuffer = (char*) malloc( measured );
        json_serialize( serializationBuffer, fd->jsonDoc );
        json_builder_free( fd->jsonDoc );
        fd->jsonDoc = fd->currentJsonNode = GUCEF_NULL;

       /*
        *   the json_measure and json_serialize functions include a null terminator
        *   When writing to a file type media it is not appropriote to include a null terminator
        */
       if ( measured > 0 )
            --measured;

        fd->fptr->write( fd->fptr, serializationBuffer, 1, (UInt32) measured );
        fd->fptr->close( fd->fptr );
    }

    if ( NULL != fd->base64EncodeBuffer )
    {
        free( fd->base64EncodeBuffer );
        fd->base64EncodeBuffer = NULL;
    }
    free( *filedata );
    *filedata = GUCEF_NULL;
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
    fd->activeNodeIsValueNode = 0;

    if ( GUCEF_NULL == fd->currentJsonNode )
    {
        if ( nodeType == GUCEF_DATATYPE_ARRAY )
        {
            fd->jsonDoc = fd->currentJsonNode = json_array_new( attscount + haschildren );
        }
        else
        {
            fd->jsonDoc = fd->currentJsonNode = json_object_new( attscount + haschildren );
        }
    }
    else
    {
        json_value* childNode = GUCEF_NULL;
        if ( nodeType == GUCEF_DATATYPE_ARRAY )
        {
            childNode = json_array_new( attscount + haschildren );
        }
        else
        if ( nodeType == GUCEF_DATATYPE_OBJECT )
        {
            if ( 1 != attscount || 0 != haschildren )
                childNode = json_object_new( attscount + haschildren );
            else
                fd->activeNodeIsValueNode = 1;
        }

        if ( GUCEF_NULL != childNode )
        {
            if ( GUCEF_NULL == nodename )
                nodename = "noname";
            if ( fd->currentJsonNode->type == json_object )
                json_object_push( fd->currentJsonNode, nodename, childNode );
            else
            if ( fd->currentJsonNode->type == json_array )
                json_array_push( fd->currentJsonNode, childNode );
            fd->currentJsonNode = childNode;
        }
        else
        {
            if ( 0 == attscount && 0 == haschildren )
            {
                TVariantData var;
                memset( &var, 0, sizeof var );
                var.containedType = nodeType;
                var.union_data.heap_data.union_data.char_heap_data = (char*) nodename;
                var.union_data.heap_data.heap_data_size = (UInt32) strlen( nodename );
                
                DSTOREPLUG_Store_Node_Att( plugdata, filedata, GUCEF_NULL, 1, 0, GUCEF_NULL, &var, haschildren );
            }
            if ( nodeType != GUCEF_DATATYPE_ARRAY && nodeType != GUCEF_DATATYPE_OBJECT )
            {
                fd->activeNodeIsValueNode = 1;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Store( void** plugdata      ,
                           void** filedata      ,
                           const char* nodename ,
                           UInt32 attscount     ,
                           UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*)*filedata;
    if ( ( GUCEF_NULL != fd->currentJsonNode ) && ( 0 == fd->activeNodeIsValueNode ) )
        fd->currentJsonNode = fd->currentJsonNode->parent;
    fd->activeNodeIsValueNode = 0;
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

    if ( 0 == attscount )
        return;

    if ( GUCEF_NULL == attvalue )
    {
        static TVariantData dummy;
        memset( &dummy, 0, sizeof dummy );
        attvalue = &dummy;
    }
    if ( GUCEF_NULL == nodename )
    {
        nodename = "noname";
    }

    switch ( attvalue->containedType )
    {
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_NIL:
        {
            json_value* att = json_null_new(); 
            if ( fd->currentJsonNode->type == json_array )
                json_array_push( fd->currentJsonNode, att );
            else
            if ( fd->currentJsonNode->type == json_object )
                if ( GUCEF_NULL != attname )
                    json_object_push( fd->currentJsonNode, attname, att );
                else
                    json_object_push( fd->currentJsonNode, nodename, att );
            break;
        }
        case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
        case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
        {
            int isNotTrue = stricmp( (const char*) attvalue->union_data.heap_data.union_data.char_heap_data, "true" );
            int isNotFalse = stricmp( (const char*) attvalue->union_data.heap_data.union_data.char_heap_data, "false" );

            json_value* att = GUCEF_NULL;
            if ( isNotTrue == 0 )
                att = json_boolean_new( 1 );
            else
            if ( isNotFalse == 0 )
                att = json_boolean_new( 0 );

            if ( att != GUCEF_NULL )
            {
                if ( fd->currentJsonNode->type == json_array )
                    json_array_push( fd->currentJsonNode, att );
                else
                if ( fd->currentJsonNode->type == json_object )
                    if ( GUCEF_NULL != attname )
                        json_object_push( fd->currentJsonNode, attname, att );
                    else
                        json_object_push( fd->currentJsonNode, nodename, att );
            }
            break;
        }
        case GUCEF_DATATYPE_BOOLEAN_INT32:
        {
            Int32 boolInt = attvalue->union_data.int32_data;
            json_value* att = json_boolean_new( boolInt );
            if ( fd->currentJsonNode->type == json_array )
                json_array_push( fd->currentJsonNode, att );
            else
            if ( fd->currentJsonNode->type == json_object )
                if ( GUCEF_NULL != attname )
                    json_object_push( fd->currentJsonNode, attname, att );
                else
                    json_object_push( fd->currentJsonNode, nodename, att );
            break;
        }
        case GUCEF_DATATYPE_FLOAT32:
        case GUCEF_DATATYPE_FLOAT64:
        case GUCEF_DATATYPE_NUMERIC:
        {
            double number = 0.0l;
            if ( GUCEF_DATATYPE_FLOAT32 == attvalue->containedType )
                number = attvalue->union_data.float32_data;
            else
                number = attvalue->union_data.float64_data;

            json_value* att = json_double_new( number );
            if ( fd->currentJsonNode->type == json_array )
                json_array_push( fd->currentJsonNode, att );
            else
            if ( fd->currentJsonNode->type == json_object )
                if ( GUCEF_NULL != attname )
                    json_object_push( fd->currentJsonNode, attname, att );
                else
                    json_object_push( fd->currentJsonNode, nodename, att );
            break;
        }
        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_INT16:
        case GUCEF_DATATYPE_UINT16:
        case GUCEF_DATATYPE_INT32:
        case GUCEF_DATATYPE_UINT32:
        case GUCEF_DATATYPE_INT64:
        case GUCEF_DATATYPE_UINT64:
        {
            json_value* att = GUCEF_NULL;
            long long integer = 0;
            switch ( attvalue->containedType )
            {
                case GUCEF_DATATYPE_INT8:   { integer = (long long) attvalue->union_data.int8_data; break; }
                case GUCEF_DATATYPE_UINT8:  { integer = (long long) attvalue->union_data.uint8_data; break; }
                case GUCEF_DATATYPE_INT16:  { integer = (long long) attvalue->union_data.int16_data; break; }
                case GUCEF_DATATYPE_UINT16: { integer = (long long) attvalue->union_data.uint16_data; break; }
                case GUCEF_DATATYPE_INT32:  { integer = (long long) attvalue->union_data.int32_data; break; }
                case GUCEF_DATATYPE_UINT32: { integer = (long long) attvalue->union_data.uint32_data; break; }
                case GUCEF_DATATYPE_INT64:  { integer = (long long) attvalue->union_data.int64_data; break; }
                case GUCEF_DATATYPE_UINT64: { integer = (long long) attvalue->union_data.uint64_data; break; }
            }

            att = json_integer_new( integer );
            if ( fd->currentJsonNode->type == json_array )
                json_array_push( fd->currentJsonNode, att );
            else
            if ( fd->currentJsonNode->type == json_object )
                if ( GUCEF_NULL != attname )
                    json_object_push( fd->currentJsonNode, attname, att );
                else
                    json_object_push( fd->currentJsonNode, nodename, att );
            break;
        }
        case GUCEF_DATATYPE_BINARY_BLOB:
        {
            if ( 0 != Base64Encode( attvalue->union_data.heap_data.union_data.void_heap_data , 
                                    attvalue->union_data.heap_data.heap_data_size            ,
                                    fd                                                       ) )
            {
                if ( GUCEF_NULL != fd->base64EncodeBuffer )
                {
                    json_value* att = json_string_new( fd->base64EncodeBuffer );
                    if ( fd->currentJsonNode->type == json_array )
                        json_array_push( fd->currentJsonNode, att );
                    else
                    if ( fd->currentJsonNode->type == json_object )
                        if ( GUCEF_NULL != attname )
                            json_object_push( fd->currentJsonNode, attname, att );
                        else
                            json_object_push( fd->currentJsonNode, nodename, att );
                    break;            
                }
            }
            break;
        }
        case GUCEF_DATATYPE_BINARY_BSOB:
        {
            if ( 0 != Base64Encode( &attvalue->union_data.bsob_data       , 
                                    sizeof attvalue->union_data.bsob_data ,
                                    fd                                    ) )
            {
                if ( GUCEF_NULL != fd->base64EncodeBuffer )
                {
                    json_value* att = json_string_new( fd->base64EncodeBuffer );
                    if ( fd->currentJsonNode->type == json_array )
                        json_array_push( fd->currentJsonNode, att );
                    else
                    if ( fd->currentJsonNode->type == json_object )
                        if ( GUCEF_NULL != attname )
                            json_object_push( fd->currentJsonNode, attname, att );
                        else
                            json_object_push( fd->currentJsonNode, nodename, att );
                    break;            
                }
            }
            break;
        }
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_ASCII_STRING:
        default:
        {
            const char* attValue = attvalue->union_data.heap_data.union_data.char_heap_data;
            if ( GUCEF_NULL == attValue )
                attValue = "";
            json_value* att = json_string_new( attValue );
            if ( fd->currentJsonNode->type == json_array )
                json_array_push( fd->currentJsonNode, att );
            else
            if ( fd->currentJsonNode->type == json_object )
                if ( GUCEF_NULL != attname )
                    json_object_push( fd->currentJsonNode, attname, att );
                else
                    json_object_push( fd->currentJsonNode, nodename, att );
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Children( void** plugdata      ,
                                void** filedata      ,
                                const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Children( void** plugdata      ,
                              void** filedata      ,
                              const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Open( void** plugdata      ,
                          void** filedata      ,
                          TIOAccess* file      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSrcFileData* sd = GUCEF_NULL;
    *plugdata = GUCEF_NULL;

    sd = (TSrcFileData*) malloc( sizeof(TSrcFileData) );
    memset( sd, 0, sizeof( TSrcFileData ) );
    sd->access = file;

    *filedata = sd;
    return 1;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Close( void** plugdata ,
                           void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( *filedata != GUCEF_NULL )
    {
        TSrcFileData* sd = (TSrcFileData*) *filedata;
        if ( GUCEF_NULL != sd->jsonDoc )
            json_value_free( sd->jsonDoc );
        sd->access->close( sd->access );
        free( sd );
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Set_Read_Handlers( void** plugdata                ,
                              void** filedata                ,
                              const TReadHandlers* rhandlers ,
                              void* privdata                 ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( *filedata != GUCEF_NULL )
    {
        TSrcFileData* sd = (TSrcFileData*) *filedata;
        sd->handlers = *rhandlers;
        sd->privdata = privdata;
    }
}

/*---------------------------------------------------------------------------*/

/* forward declaration for recursive functions below */
static void process_value( TSrcFileData* sd, const char* objName, const char* name, json_value* value );

/*---------------------------------------------------------------------------*/

static void process_object( TSrcFileData* sd, const char* name, json_value* value )
{
    int length=0, x=0, childNodes=0;
    if ( GUCEF_NULL == value )
        return;

    sd->handlers.OnNodeBegin( sd->privdata, name, GUCEF_DATATYPE_OBJECT );
    length = value->u.object.length;
    for ( x=0; x<length; x++ )
    {
        if ( value->type != json_object && value->type != json_array )
            process_value( sd, name, value->u.object.values[x].name, value->u.object.values[x].value );
        else
            childNodes++;
    }
    if ( childNodes > 0 )
    {
        sd->handlers.OnNodeChildrenBegin( sd->privdata, name );
        for ( x=0; x<length; x++ )
        {
            if ( value->type == json_object || value->type == json_array )
                process_value( sd, name, value->u.object.values[x].name, value->u.object.values[x].value );
        }
        sd->handlers.OnNodeChildrenEnd( sd->privdata, name );
    }
    sd->handlers.OnNodeEnd( sd->privdata, name );
}

/*---------------------------------------------------------------------------*/

int
json_type_to_gucef_type( json_type type )
{
    switch ( type )
    {
        case json_object:
            return GUCEF_DATATYPE_OBJECT;
        case json_array:
            return GUCEF_DATATYPE_ARRAY;
        case json_integer:
            return GUCEF_DATATYPE_INT64;
        case json_double:
            return GUCEF_DATATYPE_FLOAT64;
        case json_string:
            return GUCEF_DATATYPE_STRING;
        case json_boolean:
            return GUCEF_DATATYPE_BOOLEAN_STRING;
        case json_null:
            return GUCEF_DATATYPE_NIL;
        case json_none:
        default: 
            return GUCEF_DATATYPE_UNKNOWN;
    }
}

/*---------------------------------------------------------------------------*/

static void
process_array( TSrcFileData* sd, const char* name, json_value* value )
{
    int length=0, x=0;

    if ( GUCEF_NULL == value )
        return;

    sd->handlers.OnNodeBegin( sd->privdata, name, GUCEF_DATATYPE_ARRAY );
    sd->handlers.OnNodeChildrenBegin( sd->privdata, name );
    length = value->u.array.length;
    for ( x=0; x<length; x++ )
    {
        process_value( sd, name, GUCEF_NULL, value->u.array.values[x] );
    }
    sd->handlers.OnNodeChildrenEnd( sd->privdata, name );
    sd->handlers.OnNodeEnd( sd->privdata, name );
}

/*---------------------------------------------------------------------------*/

static void
process_value( TSrcFileData* sd    ,
               const char* objName ,
               const char* name    ,
               json_value* value   )
{
    if ( GUCEF_NULL == value )
        return;

    switch ( value->type )
    {
        case json_none:
        {
            break;
        }
        case json_null:
        {
            sd->handlers.OnNodeAtt( sd->privdata, objName, name, NULL, GUCEF_DATATYPE_NIL );
            break;
        }
        case json_object:
        {
            process_object( sd, name, value );
            break;
        }
        case json_array:
        {
            process_array( sd, name, value );
            break;
        }
        case json_integer:
        {
            char valueBuffer[64];
            sprintf( valueBuffer, "%lli", value->u.integer );

            sd->handlers.OnNodeAtt( sd->privdata, objName, name, valueBuffer, GUCEF_DATATYPE_INT64 );
            break;
        }
        case json_double:
        {
            char valueBuffer[64];
            sprintf( valueBuffer, "%f", value->u.dbl );

            sd->handlers.OnNodeAtt( sd->privdata, objName, name, valueBuffer, GUCEF_DATATYPE_FLOAT64 );
            break;
        }
        case json_string:
        {
            sd->handlers.OnNodeAtt( sd->privdata, objName, name, value->u.string.ptr, GUCEF_DATATYPE_STRING );
            break;
        }
        case json_boolean:
        {
            char* boolValue = "true";
            if ( 0 == value->u.boolean )
                boolValue = "false";

            sd->handlers.OnNodeAtt( sd->privdata, objName, name, boolValue, GUCEF_DATATYPE_BOOLEAN_STRING );
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Start_Reading( void** plugdata ,
                          void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL != *filedata )
    {
        TSrcFileData* sd = (TSrcFileData*) *filedata;
        if ( GUCEF_NULL != sd )
        {
            UInt32 fileSize = 0, bytesRead = 0;
            json_char* fileBuffer = GUCEF_NULL;
            json_value* jsonDoc = GUCEF_NULL;

            sd->access->seek( sd->access, 0, SEEK_END );
            fileSize = sd->access->tell( sd->access );
            sd->access->seek( sd->access, 0, SEEK_SET );

            fileBuffer = (json_char*) malloc( fileSize );
            bytesRead = sd->access->read( sd->access, fileBuffer, 1, fileSize );
            if ( bytesRead != fileSize )
            {
                free( fileBuffer );
                return 1;
            }

            jsonDoc = json_parse( fileBuffer, bytesRead );
            free( fileBuffer );

            free( sd->jsonDoc );
            sd->jsonDoc = jsonDoc;

            /* Now we actually commence the reading itself
               We treat the DOM tree in a SAX manner essentially */
            (*sd->handlers.OnTreeBegin)( sd->privdata );
            process_value( sd, "", "", jsonDoc );
            (*sd->handlers.OnTreeEnd)( sd->privdata );
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Type( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "json";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Name( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "gucefCORE DSTORE codec plugin utilizing json-parser for parsing and json-builder for serializing";
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
