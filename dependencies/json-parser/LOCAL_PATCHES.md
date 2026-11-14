Local patches of json-parser in GUCEF
=====================================

`json.c` differs from the upstream version it was taken from in these places, each marked with a
`GUCEF local patch` or `DV edit` comment:

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
3. **Doubles read with strtod** (`json_parse_ex`, end of a number, `DV edit`): the parser built a double from digits with
   floating point arithmetic (integer part, plus fraction divided by a power of ten, times a power of ten for the exponent), which is
   not correctly rounded: `0.30000000000000004` and `5e-324` did not come back as the double that was written. A double is now
   converted from the text of the number with `strtod` (the `.` swapped for the decimal separator of the current locale first), so
   json-builder output, which writes the shortest text that reads back, round-trips bit-identical. Upstream has the same arithmetic.

Known and left as it is: a string holding an embedded `\u0000` escape is cut at that character on its way to the
GUCEF string classes, which are zero terminated (the parser itself keeps the full length in `u.string.length`, the
DStore plugin passes it on). Callers that must keep such strings need another route.

Tests: `TestJsonDataNodeFidelity.cpp` of `gucefCORE_TestApp` (every truncation of the three literals in four
contexts, run under AddressSanitizer as well; integers beyond `Int64` after numbers with a fraction) and
`TestParquetHardening.cpp` of `tablecodecpluginPARQUET_TestApp` (table schema keys that are not strict JSON).
