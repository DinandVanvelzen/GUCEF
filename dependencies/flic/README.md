# Aseprite FLIC Library
*Copyright (C) 2015 David Capello*

> Distributed under [MIT license](LICENSE.txt)

Library to read/write Animator Pro FLI/FLC files.

## Local modifications (GUCEF)
The following changes were made to the upstream library to be able to decode the FLC animations of the game Into the Void:
- `Decoder::readHeader`: the offsets to the first and second frame in the header are only trusted if a frame header is actually found there. Some producers store positions relative to a larger archive in these fields. Otherwise decoding falls back to the sequential layout (first frame at byte 128).
- `Decoder::readHeader`: fixed `height == 0` defaulting the width instead of the height.
- `Decoder::readCopyChunk`: supports any frame size instead of only 320x200 and clamps to the size of the chunk. Remaining pixels keep their previous value.
- `Decoder::readFrame`: returns false on a bad frame magic number instead of asserting.
