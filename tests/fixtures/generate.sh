#!/bin/bash
# Regenerates the audio fixtures used by rozeTests. Needs ffmpeg.
# Every file is a 0.1 s, 440 Hz sine.
set -u
cd "$(dirname "$0")"

input=(-f lavfi -i sine=frequency=440:sample_rate=44100:duration=0.1 -ac 2)

make_fixture() {
    local out=$1
    shift
    ffmpeg -v error -y "${input[@]}" "$@" "$out" 2>&1 | head -1 | sed "s|^|$out: |"
    [ -s "$out" ] || rm -f "$out"
}

make_fixture tone.wav -c:a pcm_s16le
make_fixture tone24.wav -c:a pcm_s24le
make_fixture tonef.wav -c:a pcm_f32le
make_fixture tone.rf64 -c:a pcm_s16le -rf64 always -f wav
make_fixture tone.aiff -c:a pcm_s16be
make_fixture tone.au -c:a pcm_s16be
make_fixture tone.flac -sample_fmt s16
make_fixture tone.ogg -c:a libvorbis
make_fixture tone.opus -c:a libopus
make_fixture tone.oga -c:a flac -sample_fmt s16 -f ogg
make_fixture tone.mp3 -c:a libmp3lame
make_fixture tone.mp2 -c:a mp2
make_fixture tone.aac -c:a aac -f adts
make_fixture tone.m4a -c:a aac
make_fixture tone.m4b -c:a aac -f ipod
make_fixture tone.alac -c:a alac -sample_fmt s16p -f ipod
make_fixture tone.3gp -c:a aac -f 3gp
make_fixture tone.wv -c:a wavpack -sample_fmt s16p
make_fixture tone.tta -c:a tta -sample_fmt s16
make_fixture tone.voc -c:a pcm_u8 -ac 1
make_fixture tone.mmf -c:a adpcm_yamaha -ac 1 -ar 8000
make_fixture tone.wma -c:a wmav2
make_fixture tone.webm -c:a libopus
make_fixture tone.rm -c:a ac3 -f rm
make_fixture tone.ra -c:a real_144 -ac 1 -ar 8000 -f rm
make_fixture tone.gsm -c:a libgsm -ac 1 -ar 8000 -f gsm

# ffmpeg cannot write these (or not everywhere): build the headers by hand.
python3 - <<'PY'
import struct

# 8SVX: FORM/8SVX, VHDR (16000 Hz, no compression), BODY with 4 samples.
vhdr = struct.pack(">IIIHBBI", 4, 0, 0, 16000, 1, 0, 0x10000)
body = bytes(4)
chunks = b"VHDR" + struct.pack(">I", len(vhdr)) + vhdr + b"BODY" + struct.pack(">I", len(body)) + body
open("tone.8svx", "wb").write(b"FORM" + struct.pack(">I", 4 + len(chunks)) + b"8SVX" + chunks)

# QOA: "qoaf" <samples>, frame header <channels 1> <rate 3> <fsamples 2> <fsize 2>.
open("tone.qoa", "wb").write(b"qoaf" + struct.pack(">I", 20) + bytes([2]) + (44100).to_bytes(3, "big") + struct.pack(">HH", 20, 8))

# MOGG: <version 10> <offset of OggS>, then a real Ogg Vorbis stream.
ogg = open("tone.ogg", "rb").read()
open("tone.mogg", "wb").write(struct.pack("<II", 10, 8) + ogg)

# CDA: 44-byte RIFF/CDDA track shortcut.
fmt = struct.pack("<HHIIIII", 1, 1, 0x12345678, 0, 4410, 0, 0)
open("tone.cda", "wb").write(b"RIFF" + struct.pack("<I", 36) + b"CDDA" + b"fmt " + struct.pack("<I", 24) + fmt[:24])

# SLN and VOX have no header: any bytes work.
open("tone.sln", "wb").write(bytes(320))
open("tone.vox", "wb").write(bytes(160))
open("tone.raw", "wb").write(bytes(160))

# AMR and AMR-WB: magic then one NO_DATA frame header (Ubuntu ffmpeg has no AMR encoder).
open("tone.amr", "wb").write(b"#!AMR\n" + bytes([0x7C]))
open("tone.awb", "wb").write(b"#!AMR-WB\n" + bytes([0x7C]))
PY
