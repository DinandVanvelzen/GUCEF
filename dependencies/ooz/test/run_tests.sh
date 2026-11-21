#!/bin/bash
# Tests of the ooz_extract tool (dependencies/ooz). No other tools than bash, coreutils, xxd, grep and timeout.
#
#   run_tests.sh [path of the ooz_extract binary]
#
# Environment:
#   OOZ_EXTRACT          path of the binary (or the first parameter); default: the build directories under common/bin
#   OOZ_TEST_SCRATCH     scratch directory, created and deleted by this script (default: $HOME/AssetTests/ooz/run)
#                        keep it off a small RAM based /tmp
#   OOZ_FUZZ_ITERATIONS  number of corrupted inputs per fuzz set (default 300)
#   OOZ_TEST_PACKAGE     optional: an uncompressed zip (stored entries) or a plain file that holds a compressed
#                        buffer (magic B7 75 63 62, Oodle method 3) at some offset; enables the tests on real data
#   OOZ_TEST_ENTRY       optional: entry of OOZ_TEST_PACKAGE (a stored zip) to extract and search; when empty the
#                        package itself is searched
#
# The synthetic tests build their own streams (stored and memset quanta of the Kraken stream format) and need no data.

HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT="$( cd "$HERE/../../.." && pwd )"
BIN="${1:-$OOZ_EXTRACT}"
if [ -z "$BIN" ]; then
    for c in "$ROOT"/common/bin/CBNINJA_ooz*/bin/ooz_extract "$ROOT"/common/bin/*/bin/ooz_extract; do
        if [ -x "$c" ]; then BIN="$c"; break; fi
    done
fi
if [ -z "$BIN" ] || [ ! -x "$BIN" ]; then echo "ooz_extract binary not found, pass it as parameter or set OOZ_EXTRACT"; exit 2; fi
BIN="$( cd "$( dirname "$BIN" )" && pwd )/$( basename "$BIN" )"
SCRATCH="${OOZ_TEST_SCRATCH:-$HOME/AssetTests/ooz/run}"
ITER="${OOZ_FUZZ_ITERATIONS:-300}"
rm -rf "$SCRATCH"; mkdir -p "$SCRATCH" || exit 2
cd "$SCRATCH" || exit 2
trap 'rm -rf "$SCRATCH"' EXIT

PASS=0; FAIL=0
export GUCEF_CONSOLE_LOGGER=none

ok()   { PASS=$((PASS+1)); }
bad()  { FAIL=$((FAIL+1)); echo "FAIL: $1"; }
# expect_rc <expected exit code> <description> <command...>
expect_rc()
{
    local want="$1" what="$2"; shift 2
    timeout 60 "$@" > out.txt 2> err.txt < /dev/null
    local rc=$?
    if [ "$rc" -eq "$want" ]; then ok; else bad "$what: exit code $rc, expected $want ($(head -c 200 err.txt))"; fi
}

# Streams: header of a block: byte 0x8C (0xCC when the block is stored) and the decoder type 6 (Kraken)
mk_stored() # <file> <payload file>   stream of stored blocks of 256 KiB
{
    : > "$1"
    local size; size=$(stat -c %s "$2"); local pos=0
    while [ "$pos" -lt "$size" ]; do
        printf '\xcc\x06' >> "$1"
        dd if="$2" bs=262144 skip=$((pos/262144)) count=1 status=none >> "$1" 2>/dev/null
        pos=$((pos+262144))
    done
}
seq 1 1000 | head -c 1000 > payload1k.bin
mk_stored s1k.oz payload1k.bin
printf '\x8c\x06\x07\xff\xffA' > memset.oz         # memset quantum: 'A' repeated
for i in 1 2 3 4 5 6; do seq 1 30000; done | head -c 600000 > payload600k.bin
mk_stored s600k.oz payload600k.bin

echo "binary: $BIN"

# ---- usage ----
expect_rc 2 "no parameters" "$BIN"
expect_rc 2 "unknown parameter" "$BIN" --in s1k.oz --decompressed-size 1000 --out o.bin --bogus 1
expect_rc 2 "missing --decompressed-size" "$BIN" --in s1k.oz --out o.bin
expect_rc 2 "missing --in" "$BIN" --decompressed-size 1000 --out o.bin
expect_rc 2 "missing --out" "$BIN" --in s1k.oz --decompressed-size 1000
expect_rc 2 "value missing" "$BIN" --in s1k.oz --decompressed-size
expect_rc 2 "not a number" "$BIN" --in s1k.oz --decompressed-size 10x --out o.bin
expect_rc 2 "negative number" "$BIN" --in s1k.oz --decompressed-size -5 --out o.bin
expect_rc 2 "number overflow" "$BIN" --in s1k.oz --decompressed-size 99999999999999999999999 --out o.bin
expect_rc 2 "decompressed size zero" "$BIN" --in s1k.oz --decompressed-size 0 --out o.bin
expect_rc 2 "size zero" "$BIN" --in s1k.oz --size 0 --decompressed-size 1000 --out o.bin
expect_rc 2 "max-output zero" "$BIN" --in s1k.oz --decompressed-size 1000 --out o.bin --max-output 0
expect_rc 2 "max-output over hard limit" "$BIN" --in s1k.oz --decompressed-size 1000 --out o.bin --max-output 4294967296
expect_rc 2 "batch with --in" "$BIN" --batch --in s1k.oz
expect_rc 0 "help" "$BIN" --help

# ---- max-output ----
expect_rc 2 "decompressed size over --max-output" "$BIN" --in s1k.oz --decompressed-size 1000 --out o.bin --max-output 999
expect_rc 2 "decompressed size over the default maximum (1 GiB)" "$BIN" --in s1k.oz --decompressed-size 1073741825 --out o.bin
expect_rc 0 "decompressed size equal to --max-output" "$BIN" --in s1k.oz --decompressed-size 1000 --out o.bin --max-output 1000
expect_rc 2 "compressed size over the limit" "$BIN" --in s600k.oz --decompressed-size 1000 --out o.bin --max-output 1000

# ---- io ----
expect_rc 3 "missing input file" "$BIN" --in does-not-exist.oz --decompressed-size 1000 --out o.bin
expect_rc 3 "output in a missing directory" "$BIN" --in s1k.oz --decompressed-size 1000 --out no-such-dir/o.bin
expect_rc 3 "input is a directory" "$BIN" --in . --decompressed-size 1000 --out o.bin

# ---- synthetic streams: valid ----
rm -f o.bin; expect_rc 0 "stored stream" "$BIN" --in s1k.oz --decompressed-size 1000 --out o.bin
cmp -s o.bin payload1k.bin && ok || bad "stored stream: output differs"
[ "$(cat out.txt)" = "ok 1000" ] && ok || bad "stored stream: status line is '$(cat out.txt)'"

{ head -c 17 /dev/urandom; cat s1k.oz; head -c 33 /dev/urandom; } > s1k_embedded.bin
rm -f o.bin; expect_rc 0 "stream at offset" "$BIN" --in s1k_embedded.bin --offset 17 --size 1002 --decompressed-size 1000 --out o.bin
cmp -s o.bin payload1k.bin && ok || bad "stream at offset: output differs"
"$BIN" --in s1k_embedded.bin --offset 17 --size 1002 --decompressed-size 1000 --out - > o.bin 2> err.txt
cmp -s o.bin payload1k.bin && ok || bad "output to standard output differs"
[ "$(stat -c %s o.bin)" = "1000" ] && ok || bad "output to standard output has other bytes than the data"

rm -f o.bin; expect_rc 0 "stored stream of 3 blocks" "$BIN" --in s600k.oz --decompressed-size 600000 --out o.bin
cmp -s o.bin payload600k.bin && ok || bad "stored stream of 3 blocks: output differs"

rm -f o.bin; expect_rc 0 "memset quantum" "$BIN" --in memset.oz --decompressed-size 1000 --out o.bin
[ "$(tr -d 'A' < o.bin | wc -c)" = "0" ] && [ "$(stat -c %s o.bin)" = "1000" ] && ok || bad "memset quantum: wrong output"

# ---- synthetic streams: LZ matches that overlap at any distance ----
# One LZ quantum of the Kraken format, made by hand: 8 raw bytes, a 16 byte match 8 back, 2 literals, a 16 byte match 8
# back, then a 137 byte match <distance> back. The distance is coded in the scaled offset form (1 or 2 extra bits), the
# long match length in the length stream. Mode 1 has plain literals, mode 0 adds each literal to the byte at the last
# offset. A long match that overlaps its own output at a distance that is not a multiple of 8 (9..15, 25..31) is copied
# wrong when the copy loop is vectorized on the assumption that both pointers are 8 byte aligned.
mk_lz() # <file> <mode 0|1> <distance 8..42>
{
    local offs=$(( 8 + $3 )) sh=1
    [ "$offs" -ge 32 ] && sh=2
    local cmd=$(( ( sh << 3 ) | ( ( offs >> sh ) - 8 ) ))
    local bits=$(( ( offs & ( ( 1 << sh ) - 1 ) ) << ( 8 - sh ) ))
    local chunk=$(( 0x80 | ( $2 << 3 ) ))
    printf "\\x8c\\x06\\x00\\x00\\x1d\\x$(printf %02x $chunk)\\x00\\x1bABCDEFGH\\x00\\x00\\x02IJ\\x80\\x03\\x38\\x3a\\xfc\\x80\\x80\\x01\\x$(printf %02x $cmd)\\x80\\x01\\x78\\x$(printf %02x $bits)\\x80" > "$1"
}
lz_bytes=()
lz_match() # <distance> <length>: appends a byte by byte copy to lz_bytes
{
    local k
    for (( k=0; k<$2; k++ )); do lz_bytes+=( "${lz_bytes[${#lz_bytes[@]} - $1]}" ); done
}
lz_expected() # <file> <mode 0|1> <distance>: what mk_lz decodes to
{
    local n v
    lz_bytes=( 65 66 67 68 69 70 71 72 )
    lz_match 8 16
    for n in 73 74; do
        if [ "$2" = "0" ]; then lz_bytes+=( $(( ( n + lz_bytes[${#lz_bytes[@]} - 8] ) & 255 )) ); else lz_bytes+=( "$n" ); fi
    done
    lz_match 8 16
    lz_match "$3" 137
    for v in "${lz_bytes[@]}"; do printf "\\x$(printf %02x "$v")"; done > "$1"
}
for mode in 1 0; do
    for dist in 8 9 12 15 16 17 24 25 28 31 32 33 40; do
        mk_lz lz.oz "$mode" "$dist"; lz_expected lz.expect "$mode" "$dist"
        rm -f o.bin; expect_rc 0 "LZ quantum mode $mode, match $dist back" "$BIN" --in lz.oz --decompressed-size 179 --out o.bin
        cmp -s o.bin lz.expect && ok || bad "LZ quantum mode $mode, match $dist back: output differs"
    done
done

# ---- synthetic streams: invalid ----
rm -f o.bin
expect_rc 4 "decompressed size larger than the stream" "$BIN" --in s1k.oz --decompressed-size 1001 --out o.bin
expect_rc 4 "decompressed size smaller than the stream" "$BIN" --in s1k.oz --decompressed-size 999 --out o.bin
expect_rc 4 "size given smaller than the stream" "$BIN" --in s1k.oz --size 500 --decompressed-size 1000 --out o.bin
expect_rc 4 "range behind the end of the file" "$BIN" --in s1k.oz --offset 10 --size 5000 --decompressed-size 1000 --out o.bin
expect_rc 4 "offset behind the end of the file" "$BIN" --in s1k.oz --offset 5000 --decompressed-size 1000 --out o.bin
expect_rc 4 "trailing bytes after the stream" "$BIN" --in s1k_embedded.bin --offset 17 --decompressed-size 1000 --out o.bin
: > empty.oz; expect_rc 4 "empty input" "$BIN" --in empty.oz --decompressed-size 1000 --out o.bin
head -c 5000 /dev/urandom > garbage.bin; expect_rc 4 "random garbage" "$BIN" --in garbage.bin --decompressed-size 1000 --out o.bin
[ ! -e o.bin ] && ok || bad "an output file was written for a failed request"
printf '\x8c\x06\x00' > cut.oz; expect_rc 4 "stream cut in the header" "$BIN" --in cut.oz --decompressed-size 1000 --out o.bin

# ---- batch protocol ----
mkdir -p "dir with space"; cp s1k.oz "dir with space/in 1.oz"
cat > batch_in.txt <<EOF
--in s1k.oz --decompressed-size 1000 --out b1.bin
--in "dir with space/in 1.oz" --decompressed-size 1000 --out "b 2.bin"

--in s1k.oz --decompressed-size 1001 --out b3.bin
--in missing.oz --decompressed-size 1000 --out b4.bin
--bogus 1
--in s1k.oz --decompressed-size 1000 --out -
--in s1k_embedded.bin --offset 17 --size 1002 --decompressed-size 1000 --out b5.bin
--in "unterminated --decompressed-size 1000 --out b6.bin
--in memset.oz --decompressed-size 1000 --out b7.bin --max-output 999
--in memset.oz --decompressed-size 1000 --out b8.bin
EOF
timeout 60 "$BIN" --batch < batch_in.txt > batch_out.txt 2> err.txt; rc=$?
[ "$rc" -eq 0 ] && ok || bad "batch exit code $rc"
[ "$(wc -l < batch_out.txt)" = "10" ] && ok || bad "batch: $(wc -l < batch_out.txt) status lines, expected 10"
expect_line() { sed -n "$1p" batch_out.txt | grep -q "^$2" && ok || bad "batch status line $1: '$(sed -n "$1p" batch_out.txt)', expected '$2...'"; }
expect_line 1 "ok 1000$"
expect_line 2 "ok 1000$"
expect_line 3 "error 4 "
expect_line 4 "error 3 "
expect_line 5 "error 2 "
expect_line 6 "error 2 "
expect_line 7 "ok 1000$"
expect_line 8 "error 2 "
expect_line 9 "error 2 "
expect_line 10 "ok 1000$"
cmp -s b1.bin payload1k.bin && cmp -s "b 2.bin" payload1k.bin && cmp -s b5.bin payload1k.bin && ok || bad "batch outputs differ"
[ ! -e b3.bin ] && [ ! -e b4.bin ] && [ ! -e b7.bin ] && ok || bad "batch wrote files for failed requests"
# every request line is answered, also after an error
timeout 60 "$BIN" --batch --max-output 500 < batch_in.txt > batch_out2.txt 2> err.txt
sed -n 1p batch_out2.txt | grep -q "^error 2 " && ok || bad "batch: --max-output of the command line is the default of the requests"

# liveness and flushing: answer arrives before the next request is sent, the process lives until stdin closes
mkfifo fifo_in fifo_out
"$BIN" --batch < fifo_in > fifo_out 2>/dev/null &
BPID=$!
exec 7> fifo_in; exec 8< fifo_out
echo "--in s1k.oz --decompressed-size 1000 --out l1.bin" >&7
read -t 10 -u 8 line1; [ "$line1" = "ok 1000" ] && ok || bad "batch live: first answer '$line1'"
echo "--in nope.oz --decompressed-size 1000 --out l2.bin" >&7
read -t 10 -u 8 line2; [[ "$line2" == "error 3 "* ]] && ok || bad "batch live: second answer '$line2'"
kill -0 $BPID 2>/dev/null && ok || bad "batch live: process ended before stdin closed"
echo "--in s1k.oz --decompressed-size 1000 --out l3.bin" >&7
read -t 10 -u 8 line3; [ "$line3" = "ok 1000" ] && ok || bad "batch live: third answer '$line3'"
exec 7>&-
for i in 1 2 3 4 5 6 7 8 9 10; do kill -0 $BPID 2>/dev/null || break; sleep 0.2; done
if kill -0 $BPID 2>/dev/null; then bad "batch live: process did not end when stdin closed"; kill $BPID; fi
wait $BPID; [ $? -eq 0 ] && ok || bad "batch live: exit code of the process was not 0"
exec 8<&-; rm -f fifo_in fifo_out

# a very long line and many lines
{ printf -- '--in '; head -c 100000 /dev/zero | tr '\0' 'a'; printf '\n--in s1k.oz --decompressed-size 1000 --out ll.bin\n'; } > long_in.txt
timeout 60 "$BIN" --batch < long_in.txt > long_out.txt 2>/dev/null
[ "$(sed -n 1p long_out.txt | cut -c1-8)" = "error 2 " ] && [ "$(sed -n 2p long_out.txt)" = "ok 1000" ] && ok || bad "batch: long line handling: $(head -c 100 long_out.txt)"

# ---- corrupted input: no crash, no hang ----
# Every request must end with 0, 4 (corrupt or truncated, including a decoder that crashed or timed out) or 5; any
# other exit code (a signal of this process, a hang that timeout cuts off) is a failure.
fuzz()
{
    local src="$1" size="$2" dec="$3" name="$4" n="$5" rc crashes=0 oks=0 corrupt=0
    local seed=12345
    RANDOM=$seed
    for ((i=0; i<n; i++)); do
        cp "$src" f.bin
        local mode=$((i % 3)) pos
        if [ $mode -eq 0 ]; then            # truncate
            pos=$(( (RANDOM*32768 + RANDOM) % size ))
            head -c "$pos" "$src" > f.bin
        elif [ $mode -eq 1 ]; then          # flip bits
            local flips=$(( 1 + RANDOM % 8 ))
            for ((k=0; k<flips; k++)); do
                pos=$(( (RANDOM*32768 + RANDOM) % size ))
                local old; old=$(xxd -s "$pos" -l 1 -p f.bin)
                local new; new=$(printf '%02x' $(( 16#$old ^ (1 << (RANDOM % 8)) )))
                printf "$(printf '\\x%s' "$new")" | dd of=f.bin bs=1 seek="$pos" conv=notrunc status=none
            done
        else                                # overwrite a few bytes with random ones in the first 64 bytes
            pos=$(( RANDOM % 64 )); [ $pos -ge "$size" ] && pos=0
            head -c 4 /dev/urandom | dd of=f.bin bs=1 seek="$pos" conv=notrunc status=none
        fi
        timeout 30 "$BIN" --timeout 10 --in f.bin --decompressed-size "$dec" --out fo.bin > /dev/null 2> ferr.txt < /dev/null
        rc=$?
        case $rc in
            0) oks=$((oks+1));;
            4) corrupt=$((corrupt+1)); grep -q "crashed" ferr.txt && crashes=$((crashes+1));;
            5) corrupt=$((corrupt+1));;
            *) bad "fuzz $name iteration $i: exit code $rc"; cp f.bin "fuzz_fail_${name}_$i.bin";;
        esac
    done
    ok
    echo "fuzz $name: $n inputs, $oks decoded, $corrupt rejected (of which the decoder crashed: $crashes)"
}
fuzz s600k.oz $(stat -c %s s600k.oz) 600000 stored "$ITER"
# a batch run over many corrupted inputs must stay alive and answer every line
: > fb_in.txt; for ((i=0; i<100; i++)); do
    head -c $(( 3 + (i*7) % 600 )) /dev/urandom > "fb_$i.bin"; printf '\x8c\x06' | dd of="fb_$i.bin" conv=notrunc status=none
    echo "--in fb_$i.bin --decompressed-size $(( 1000 + i*977 )) --out fbo.bin" >> fb_in.txt
done
timeout 120 "$BIN" --batch < fb_in.txt > fb_out.txt 2>/dev/null; rc=$?
[ "$rc" -eq 0 ] && [ "$(wc -l < fb_out.txt)" = "100" ] && [ "$(grep -vc '^\(ok\|error [2345]\) ' fb_out.txt)" = "0" ] && ok || bad "batch over 100 garbage inputs: exit code $rc, $(wc -l < fb_out.txt) lines"

# ---- real data (optional) ----
if [ -n "$OOZ_TEST_PACKAGE" ] && [ -r "$OOZ_TEST_PACKAGE" ]; then
    DATA="$OOZ_TEST_PACKAGE"
    if [ -n "$OOZ_TEST_ENTRY" ]; then
        mkdir -p real && unzip -o -q "$OOZ_TEST_PACKAGE" "$OOZ_TEST_ENTRY" -d real && DATA="real/$OOZ_TEST_ENTRY"
    fi
    hexat() { xxd -p -s "$2" -l "$3" "$1" | tr -d '\n'; }
    MAGIC=$(LC_ALL=C grep -obUaP '\xB7\x75\x63\x62' "$DATA" | head -1 | cut -d: -f1)
    if [ -z "$MAGIC" ]; then echo "real data: no compressed buffer found in $DATA, skipped"; else
        H="$(hexat "$DATA" "$MAGIC" 64)"
        METHOD=$((16#${H:16:2})); EXP=$((16#${H:22:2})); COUNT=$((16#${H:24:8})); RAW=$((16#${H:32:16})); COMP=$((16#${H:48:16}))
        BS=$((1 << EXP))
        echo "real data: buffer at $MAGIC, method $METHOD, $COUNT blocks of $BS, raw $RAW, compressed $COMP"
        [ "$METHOD" -eq 3 ] && ok || bad "real data: method $METHOD is not Oodle (3)"
        SIZES="$(hexat "$DATA" $((MAGIC+64)) $((COUNT*4)))"
        POS=$((MAGIC + 64 + COUNT*4)); LEFT=$RAW
        : > real_req.txt; : > real_all.bin
        for ((b=0; b<COUNT; b++)); do
            CS=$((16#${SIZES:b*8:8})); RS=$BS; [ "$LEFT" -lt "$RS" ] && RS=$LEFT
            echo "--in \"$DATA\" --offset $POS --size $CS --decompressed-size $RS --out \"real_b$b.bin\"" >> real_req.txt
            POS=$((POS+CS)); LEFT=$((LEFT-RS))
        done
        [ $((POS - MAGIC)) -eq "$COMP" ] && ok || bad "real data: header and block sizes do not add up to the total compressed size of the buffer"
        timeout 600 "$BIN" --batch < real_req.txt > real_out.txt 2>/dev/null
        [ "$(grep -c '^ok ' real_out.txt)" = "$COUNT" ] && ok || bad "real data: $(grep -c '^ok ' real_out.txt) of $COUNT blocks decoded: $(grep -v '^ok ' real_out.txt | head -3)"
        for ((b=0; b<COUNT; b++)); do cat "real_b$b.bin" >> real_all.bin 2>/dev/null; done
        [ "$(stat -c %s real_all.bin)" = "$RAW" ] && ok || bad "real data: decompressed $(stat -c %s real_all.bin) bytes, expected $RAW"
        # the data of the mesh payloads starts with a length prefixed name string
        if [ "$(head -c 16 real_all.bin | tail -c 8)" = "Vertices" ]; then ok; else echo "real data: note: payload does not start with the expected name string ($(head -c 16 real_all.bin | xxd -p))"; fi
        # single request mode and standard output give the same bytes as the batch run
        CS0=$((16#${SIZES:0:8}))
        "$BIN" --in "$DATA" --offset $((MAGIC+64+COUNT*4)) --size "$CS0" --decompressed-size "$BS" --out - 2>/dev/null | cmp -s - real_b0.bin && ok || bad "real data: single request differs from batch"
        # the whole buffer is not one stream: the blocks are separate streams, decoding block 0 with the wrong size must fail
        expect_rc 4 "real data: block 0 with a size one larger" "$BIN" --in "$DATA" --offset $((MAGIC+64+COUNT*4)) --size "$CS0" --decompressed-size $((BS+1)) --out real_x.bin
        # corrupted real blocks
        dd if="$DATA" of=real_blk0.oz bs=1 skip=$((MAGIC+64+COUNT*4)) count="$CS0" status=none
        fuzz real_blk0.oz "$CS0" "$BS" real "$ITER"
    fi
else
    echo "real data tests skipped (set OOZ_TEST_PACKAGE, and OOZ_TEST_ENTRY for a zip)"
fi

echo "passed: $PASS, failed: $FAIL"
[ "$FAIL" -eq 0 ]
