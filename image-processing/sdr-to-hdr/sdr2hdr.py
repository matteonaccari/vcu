'''
SDR to HRD mapping (sdr2hdr), version 1.0
Copyright(c) 2021 Matteo Naccari
All Rights Reserved.

email: matteo.naccari@gmail.com | matteo.naccari@polimi.it | matteo.naccari@lx.it.pt
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the author may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

The sdr2hdr software converts Standard Dynamic Range (SDR) material to its High
Dynamic Range (HDR) counterpart using the of Hybrid Log-Gamma (HLG) transfer
characteristics. The whole conversion process is described in greater detail in
ITU-R BT.2408 report, available here:
https://www.itu.int/dms_pub/itu-r/opb/rep/R-REP-BT.2408-4-2021-PDF-E.pdf

In particular the "Scene referred mapping of SDR into HLG" method is implemented.
Please note that the conversion merely places an SDR content into an HDR-HLG one,
assuming that 100% of the SDR signal is mapped into 75% HDR. Accordingly, no re-grading
is performed here. More information is also available from:
https://jvet-experts.org/doc_end_user/documents/7_Torino/wg11/JVET-G0059-v2.zip

Input signals are assumed in the YCbCr colour space with ITU-R BT.709 primaries,
4:4:4 chroma format and in the video range. Input bit depths of 8 and 10 bits are supported.

Parameters:
    - Input:
        in_file_name     = Name of the input video sequence.
        out_file_name    = Name of the output video sequence.
        rows             = Height of each video frame in pixel units.
        cols             = Width of each video frame in pixel units.
        bit_depth        = Input bit depth (8 or 10, 8 is the default).
        frames           = Number of frames to be processed (default all).
'''


import sys
from argparse import ArgumentParser
from typing import Any, Tuple

import numpy as np
from nptyping import NDArray


def inverse_quantisation(y: NDArray[(Any, Any), np.float32], cb: NDArray[(Any, Any), np.float32],
                         cr: NDArray[(Any, Any), np.float32],
                         bit_depth: int
                         ) -> Tuple[NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32]]:

    scaling = 1 << (bit_depth - 8)
    y_n = np.clip((y / scaling - 16) / 219, 0, 1.0)
    cb_n = np.clip((cb / scaling - 128) / 224, -0.5, 0.5)
    cr_n = np.clip((cr / scaling - 128) / 224, -0.5, 0.5)

    return y_n, cb_n, cr_n


def ycbcr_to_rgb_bt709(y: NDArray[(Any, Any), np.float32], cb: NDArray[(Any, Any), np.float32], cr: NDArray[(Any, Any), np.float32]
                       ) -> Tuple[NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32]]:

    T = np.array([[1.0, 0.0, 1.57480], [1.0, -0.18733, -0.46813], [1.0, 1.85563, 0.0]])

    r = np.clip(T[0][0] * y + T[0][1] * cb + T[0][2] * cr, 0, 1)
    g = np.clip(T[1][0] * y + T[1][1] * cb + T[1][2] * cr, 0, 1)
    b = np.clip(T[2][0] * y + T[2][1] * cb + T[2][2] * cr, 0, 1)

    return r, g, b


def bt1886_eotf(r: NDArray[(Any, Any), np.float32], g: NDArray[(Any, Any), np.float32], b: NDArray[(Any, Any), np.float32],
                display_gamma: float, lw: float, lb: float) -> Tuple[NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32]]:

    argument = lw ** (1 / display_gamma) - lb ** (1 / display_gamma)
    alpha = argument ** display_gamma
    beta = lb ** (1 / display_gamma) / argument
    max_value = alpha * (1 + beta) ** display_gamma

    r_ll = alpha * np.power(np.maximum(0, r + beta), display_gamma) / max_value
    g_ll = alpha * np.power(np.maximum(0, g + beta), display_gamma) / max_value
    b_ll = alpha * np.power(np.maximum(0, b + beta), display_gamma) / max_value

    return r_ll, g_ll, b_ll


def rgb709_to_rgb2020(r709: NDArray[(Any, Any), np.float32], g709: NDArray[(Any, Any), np.float32], b709: NDArray[(Any, Any), np.float32]
                      ) -> Tuple[NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32]]:

    T = np.array([
        [0.627404078626, 0.329282097415, 0.043313797587],
        [0.069097233123, 0.919541035593, 0.011361189924],
        [0.016391587664, 0.088013255546, 0.895595009604]])

    r2020 = np.clip(r709 * T[0][0] + g709 * T[0][1] + b709 * T[0][2], 0, 1)
    g2020 = np.clip(r709 * T[1][0] + g709 * T[1][1] + b709 * T[1][2], 0, 1)
    b2020 = np.clip(r709 * T[2][0] + g709 * T[2][1] + b709 * T[2][2], 0, 1)

    return r2020, g2020, b2020


def scaling_scene_referred(r: NDArray[(Any, Any), np.float32], g: NDArray[(Any, Any), np.float32], b: NDArray[(Any, Any), np.float32]
                           ) -> Tuple[NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32]]:

    scale = 0.2546
    system_gamma = 1.03

    r_s = (scale * r) ** (1 / system_gamma)
    g_s = (scale * g) ** (1 / system_gamma)
    b_s = (scale * b) ** (1 / system_gamma)

    return r_s, g_s, b_s


def hlg_oetf(r_s: NDArray[(Any, Any), np.float32], g_s: NDArray[(Any, Any), np.float32], b_s: NDArray[(Any, Any), np.float32]
             ) -> Tuple[NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32], NDArray[(Any, Any), np.float32]]:

    a, b, c = 0.17883277, 0.28466892, 0.55991073
    splice_point = 1 / 12

    r_hlg, g_hlg, b_hlg = np.zeros_like(r_s), np.zeros_like(g_s), np.zeros_like(b_s)

    # Red
    below_splice, above_splice = np.where(r_s <= splice_point), np.where(r_s > splice_point)
    r_hlg[below_splice] = np.sqrt(3 * r_s[below_splice])
    r_hlg[above_splice] = a * np.log(12 * r_s[above_splice] - b) + c

    # Green
    below_splice, above_splice = np.where(g_s <= splice_point), np.where(g_s > splice_point)
    g_hlg[below_splice] = np.sqrt(3 * g_s[below_splice])
    g_hlg[above_splice] = a * np.log(12 * g_s[above_splice] - b) + c

    # Blue
    below_splice, above_splice = np.where(b_s <= splice_point), np.where(b_s > splice_point)
    b_hlg[below_splice] = np.sqrt(3 * b_s[below_splice])
    b_hlg[above_splice] = a * np.log(12 * b_s[above_splice] - b) + c

    return r_hlg, g_hlg, b_hlg


def rgb_to_quant_ycbcr(r: NDArray[(Any, Any), np.float32], g: NDArray[(Any, Any), np.float32], b: NDArray[(Any, Any), np.float32],
                       bit_depth: int) -> Tuple[NDArray[(Any, Any), np.int16], NDArray[(Any, Any), np.int16], NDArray[(Any, Any), np.int16]]:
    M = np.array([[0.2627, 0.6780, 0.0593], [-0.13963, -0.36037, 0.5], [0.5, -0.459786, -0.040214]], np.float32)
    max_value = (1 << bit_depth) - 1
    scaling_factor = 1 << (bit_depth - 8)

    y_n = scaling_factor * (219 * (M[0][0] * r + M[0][1] * g + M[0][2] * b) + 16)
    cb_n = scaling_factor * (224 * (M[1][0] * r + M[1][1] * g + M[1][2] * b) + 128)
    cr_n = scaling_factor * (224 * (M[2][0] * r + M[2][1] * g + M[2][2] * b) + 128)

    y = np.clip(y_n, 0, max_value).astype(np.int16)
    cb = np.clip(cb_n, 0, max_value).astype(np.int16)
    cr = np.clip(cr_n, 0, max_value).astype(np.int16)

    return y, cb, cr


def sdr2hdr(in_file_name: str, out_file_name: str, rows: int, cols: int, bit_depth: int = 8, frames: int = -1) -> None:
    if bit_depth not in (8, 10):
        raise Exception("Bit depth must 8 or 10")

    bytes_sample = 1 if bit_depth == 8 else 2
    bytes_frame = (3 * rows * cols * bytes_sample)
    total_pixels = rows * cols
    precision = np.uint8 if bit_depth == 8 else np.uint16
    with open(in_file_name, 'rb') as in_fh, open(out_file_name, 'wb') as out_fh:
        in_fh.seek(0, 2)
        position = in_fh.tell()
        total_frames = position // bytes_frame

        in_fh.seek(0, 0)

        if frames != -1:
            frames = min(frames, total_frames)
        else:
            frames = total_frames

        for frame_idx in range(frames):
            print(f"Processing frame: {frame_idx:05d}")

            # Read the YCbCr components
            Y = np.reshape(np.frombuffer(in_fh.read(total_pixels), dtype=precision), (rows, cols)).astype(np.float32)
            Cb = np.reshape(np.frombuffer(in_fh.read(total_pixels), dtype=precision), (rows, cols)).astype(np.float32)
            Cr = np.reshape(np.frombuffer(in_fh.read(total_pixels), dtype=precision), (rows, cols)).astype(np.float32)

            # Bring them back to the normalised range [0, 1] and [-0.5, 0.5] for chroma
            y_n, cb_n, cr_n = inverse_quantisation(Y, Cb, Cr, bit_depth)

            # Convert the YCbCr to the RGB colour space with BT.709 colour primaries
            r_n, g_n, b_n = ycbcr_to_rgb_bt709(y_n, cb_n, cr_n)

            # Apply the BT.1886 EOTF
            r_ll, g_ll, b_ll = bt1886_eotf(r_n, g_n, b_n, 2.4, 100, 0.1)

            # Convert RGB BT.709 to RGB BT.2020 colour primaries
            r2020, g2020, b2020 = rgb709_to_rgb2020(r_ll, g_ll, b_ll)

            # Perform the scaling to obtain scene referred material
            r_s, g_s, b_s = scaling_scene_referred(r2020, g2020, b2020)

            # Apply the BT.2100 HLG OETF
            r_hlg, g_hlg, b_hlg = hlg_oetf(r_s, g_s, b_s)

            # Convert RGB to YUV using BT.2020 colour primaries and apply quantisation
            y_hlg, cb_hlg, cr_hlg = rgb_to_quant_ycbcr(r_hlg, g_hlg, b_hlg, 10)

            out_fh.write(y_hlg.tobytes())
            out_fh.write(cb_hlg.tobytes())
            out_fh.write(cr_hlg.tobytes())


if __name__ == "__main__":
    cmdline_parser = ArgumentParser(description="Perform the SDR to HDR video conversion")
    cmdline_parser.add_argument("--input", type=str, required=True, help="Input file in YCbCr planar and 4:4:4 chroma format")
    cmdline_parser.add_argument("--output", type=str, required=True, help="Output file in YCbCr planar and 4:4:4 chroma format")
    cmdline_parser.add_argument("--height", type=int, required=True, help="Frame height")
    cmdline_parser.add_argument("--width", type=int, required=True, help="Frame width")
    cmdline_parser.add_argument("--bit-depth", type=int, default=8, help="Input bit depth")
    cmdline_parser.add_argument("--frames", type=int, default=-1, help="Total number of frames to be processed, input -1 for all frames")

    args = cmdline_parser.parse_args()

    if len(sys.argv) < 9:
        cmdline_parser.print_help()
        sys.exit(0)

    sdr2hdr(args.input, args.output, args.height, args.width, args.bit_depth, args.frames)
