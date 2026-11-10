Local patches of json-parser in GUCEF
=====================================

`json.c` differs from the upstream version it was taken from in these places, each marked with a
`GUCEF local patch` comment:

1. **Literal bounds** (`json_parse_ex`, cases `'t'`, `'f'` and `'n'`): the checks were `(end - state.ptr) < 3`
   for `true` and `null` and `< 4` for `false`, one byte short: a text that ends inside a literal (`[tru`, `[fals`,
   `[nul`) made the parser read the byte after the end of the text. The checks are now `<= 3` and `<= 4`, so the
   whole literal must lie within the text. The DStore JSON plugin passes buffers of exactly the text length, so the
   read went past the heap block (found with AddressSanitizer through the `gucef.table.schema` key of a parquet
   footer).
2. **Number flags** (`json_parse_ex`, start of a number in the second pass): `flag_num_got_decimal` was not
   reset with the other number flags. After any number with a fraction, an integer too large for `Int64` (which
   the parser continues as a double) took its remaining digits as a fraction and came out ten or more times too
   small. The flag is now reset with the others.

Known and left as it is: a string holding an embedded `\u0000` escape is cut at that character on its way to the
GUCEF string classes, which are zero terminated (the parser itself keeps the full length in `u.string.length`, the
DStore plugin passes it on). Callers that must keep such strings need another route.

Tests: `TestJsonDataNodeFidelity.cpp` of `gucefCORE_TestApp` (every truncation of the three literals in four
contexts, run under AddressSanitizer as well; integers beyond `Int64` after numbers with a fraction) and
`TestParquetHardening.cpp` of `tablecodecpluginPARQUET_TestApp` (table schema keys that are not strict JSON).
