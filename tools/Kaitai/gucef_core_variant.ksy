#
#   gucefCORE: GUCEF module providing O/S abstraction and generic solutions
#
#   Copyright (C) 1998 - 2022.  Dinand Vanvelzen
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
meta:
  id: gucef_core_variant
  endian: le
  imports:
  - gucef_core_dynamic
seq:
  - id: type_id
    type: u1
    enum: gucef_datatypes
  - id: payload    
    type:
      switch-on: type_id
      cases:
        'gucef_datatypes::uint8': gucef_uint8
        'gucef_datatypes::int8': gucef_int8
        'gucef_datatypes::le_uint16': gucef_le_uint16
        'gucef_datatypes::be_uint16': gucef_be_uint16
        'gucef_datatypes::le_int16': gucef_le_int16
        'gucef_datatypes::be_int16': gucef_be_int16
        'gucef_datatypes::le_uint32': gucef_le_uint32
        'gucef_datatypes::be_uint32': gucef_be_uint32
        'gucef_datatypes::le_int32': gucef_le_int32
        'gucef_datatypes::be_int32': gucef_be_int32
        'gucef_datatypes::le_uint64': gucef_le_uint64
        'gucef_datatypes::be_uint64': gucef_be_uint64
        'gucef_datatypes::le_int64': gucef_le_int64
        'gucef_datatypes::be_int64': gucef_be_int64
        'gucef_datatypes::le_float32': gucef_le_float32
        'gucef_datatypes::be_float32': gucef_be_float32
        'gucef_datatypes::le_float64': gucef_le_float64
        'gucef_datatypes::be_float64': gucef_be_float64
        'gucef_datatypes::le_boolean_int32': gucef_le_boolean_int32
        'gucef_datatypes::be_boolean_int32': gucef_be_boolean_int32
        'gucef_datatypes::binary_bsob': gucef_bsob
        'gucef_datatypes::binary_blob': gucef_core_dynamic
        'gucef_datatypes::le_timestamp_in_secs_since_unix_epoch': gucef_le_timestamp_in_secs_since_unix_epoch
        'gucef_datatypes::be_timestamp_in_secs_since_unix_epoch': gucef_be_timestamp_in_secs_since_unix_epoch
        'gucef_datatypes::le_timestamp_in_ms_since_unix_epoch': gucef_le_timestamp_in_ms_since_unix_epoch
        'gucef_datatypes::be_timestamp_in_ms_since_unix_epoch': gucef_be_timestamp_in_ms_since_unix_epoch
        'gucef_datatypes::le_fraction_int32t2': gucef_le_fraction_int32t2
        'gucef_datatypes::be_fraction_int32t2': gucef_be_fraction_int32t2
        'gucef_datatypes::le_fraction_uint32t2': gucef_le_fraction_uint32t2
        'gucef_datatypes::be_fraction_uint32t2': gucef_be_fraction_uint32t2
        'gucef_datatypes::le_fraction_int64_int32': gucef_le_fraction_int64_int32
        'gucef_datatypes::be_fraction_int64_int32': gucef_be_fraction_int64_int32
        'gucef_datatypes::le_fraction_uint64_uint32': gucef_le_fraction_uint64_uint32
        'gucef_datatypes::be_fraction_uint64_uint32': gucef_be_fraction_uint64_uint32
        'gucef_datatypes::le_fraction_int64t2': gucef_core_dynamic
        'gucef_datatypes::be_fraction_int64t2': gucef_core_dynamic
        'gucef_datatypes::le_fraction_uint64t2': gucef_core_dynamic
        'gucef_datatypes::be_fraction_uint64t2': gucef_core_dynamic
        'gucef_datatypes::boolean_ascii_string' : gucef_core_dynamic
        'gucef_datatypes::boolean_utf8_string' : gucef_core_dynamic
        'gucef_datatypes::ascii_string' : gucef_core_dynamic
        'gucef_datatypes::utf8_string' : gucef_core_dynamic
        'gucef_datatypes::utf16_le_string' : gucef_core_dynamic
        'gucef_datatypes::utf16_be_string' : gucef_core_dynamic
        'gucef_datatypes::utf32_string' : gucef_core_dynamic
        'gucef_datatypes::datetime_iso8601_ascii_string' : gucef_core_dynamic
        'gucef_datatypes::datetime_iso8601_utf8_string' : gucef_core_dynamic
enums:
  gucef_datatypes:
    1: uint8
    2: int8
    3: le_uint16
    4: be_uint16
    5: le_int16
    6: be_int16
    7: le_uint32
    8: be_uint32
    9: le_int32
    10: be_int32
    11: le_uint64
    12: be_uint64
    13: le_int64
    14: be_int64
    15: le_float32
    16: be_float32
    17: le_float64
    18: be_float64
    21: ascii_string
    22: utf8_string
    23: utf16_le_string
    24: utf16_be_string
    25: utf32_le_string
    26: utf32_be_string
    30: boolean_int32
    31: boolean_ascii_string
    32: boolean_utf8_string
    33: boolean_utf16_le_string
    34: boolean_utf16_be_string
    35: boolean_utf32_le_string
    36: boolean_utf32_be_string
    40: binary_blob
    41: binary_bsob
    50: datetime_iso8601_ascii_string
    51: datetime_iso8601_utf8_string
    55: le_timestamp_in_secs_since_unix_epoch
    56: be_timestamp_in_secs_since_unix_epoch
    57: le_timestamp_in_ms_since_unix_epoch
    58: be_timestamp_in_ms_since_unix_epoch
    60: le_fraction_int32t2
    61: be_fraction_int32t2
    62: le_fraction_uint32t2
    63: be_fraction_uint32t2
    64: le_fraction_int64t2
    65: be_fraction_int64t2
    66: le_fraction_uint64t2
    67: be_fraction_uint64t2
    68: le_fraction_int64_int32
    69: be_fraction_int64_int32
    70: le_fraction_uint64_uint32
    71: be_fraction_uint64_uint32
types:
  gucef_uint8:
    seq:
        - id: data
          type: u1
  gucef_int8:
    seq:
        - id: data
          type: s1
  gucef_le_uint16:
    seq:
        - id: data
          type: u2
  gucef_be_uint16:
    seq:
        - id: data
          type: u2
          endian: be
  gucef_le_int16:
    seq:
        - id: data
          type: s2
  gucef_be_int16:
    seq:
        - id: data
          type: s2
          endian: be
  gucef_le_uint32:
    seq:
        - id: data
          type: u4
  gucef_be_uint32:
    seq:
        - id: data
          type: u4
          endian: be
  gucef_le_int32:
    seq:
        - id: data
          type: s4
  gucef_be_int32:
    seq:
        - id: data
          type: s4
          endian: be
  gucef_le_uint64:
    seq:
        - id: data
          type: u8
  gucef_be_uint64:
    seq:
        - id: data
          type: u8
          endian: be
  gucef_le_int64:
    seq:
        - id: data
          type: s8
  gucef_be_int64:
    seq:
        - id: data
          type: s8
          endian: be
  gucef_le_float32:
    seq:
        - id: data
          type: f4
  gucef_be_float32:
    seq:
        - id: data
          type: f4
          endian: be
  gucef_le_float64:
    seq:
        - id: data
          type: f8
  gucef_be_float64:
    seq:
        - id: data
          type: f8
          endian: be
  gucef_le_boolean_int32:
    seq:
        - id: data
          type: s4
  gucef_be_boolean_int32:
    seq:
        - id: data
          type: s4
          endian: be
  gucef_bsob:
    seq:
        - id: data
          type: u1
  gucef_le_timestamp_in_secs_since_unix_epoch:
    seq:
        - id: data
          type: u8
  gucef_be_timestamp_in_secs_since_unix_epoch:
    seq:
        - id: data
          type: u8
          endian: be
  gucef_le_timestamp_in_ms_since_unix_epoch:
    seq:
        - id: data
          type: u8
  gucef_be_timestamp_in_ms_since_unix_epoch:
    seq:
        - id: data
          type: u8
          endian: be
  gucef_le_fraction_int32t2:
    seq:
        - id: numerator
          type: s4                
        - id: denominator
          type: s4
  gucef_be_fraction_int32t2:
    seq:
        - id: numerator
          type: s4
          endian: be
        - id: denominator
          type: s4
          endian: be
  gucef_le_fraction_uint32t2:
    seq:
        - id: numerator
          type: u4                
        - id: denominator
          type: u4
  gucef_be_fraction_uint32t2:
    seq:
        - id: numerator
          type: u4
          endian: be
        - id: denominator
          type: u4
          endian: be
  gucef_le_fraction_int64_int32:
    seq:
        - id: numerator
          type: s8                
        - id: denominator
          type: s4
  gucef_be_fraction_int64_int32:
    seq:
        - id: numerator
          type: s8
          endian: be
        - id: denominator
          type: s4
          endian: be
  gucef_le_fraction_uint64_uint32:
    seq:
        - id: numerator
          type: u8
        - id: denominator
          type: u4
  gucef_be_fraction_uint64_uint32:
    seq:
        - id: numerator
          type: u8
          endian: be
        - id: denominator
          type: u4
          endian: be