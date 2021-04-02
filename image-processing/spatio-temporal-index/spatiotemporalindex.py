'''
Spatial Index (SI) and Temporal Index (TI) calculator, version 1.0
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

The spatiotemporalindex software returns the SI and TI information
associated to the video content whose YCbCr file name is passed as input
parameter. The spatial and temporal information is computed according
to the specification given in: ITU-T, "Subjective video quality
assessment methods for multimedia applications", Recommendation ITU-T
P 910, September 1999.
WARNING: When a YCbCr is presented as input, planar format is assumed

Parameters:
    - Input:
        sequence_name   = string containing the name of the YCbCr video
                         sequence whereby a 4:2:0 chrominance
                         subsampling factor is assumed
        R               = number of rows for each video frame
        C               = number of columns for each video frame
        chroma_format   = chroma subsampling format (4:0:0, 4:2:0, 4:2:2 or 4:4:4)
        input_bit_depth = sample bit depth
        frame_range     = range/number of frames to be processed. Range of frames are
                          expressed as start_idx:stop_idx where the index starts from zero

    - Output:
        SI              = variable containing the SI associated to the
                          video content
        SI              = variable containing the TI associated to the
                          video content
'''

from typing import Any, Tuple

import numpy as np
import scipy.signal as dsp
from nptyping import NDArray


def spatiotemporalindex(sequence_name: str, R: int, C: int, chroma_format: str, input_bit_depth: int, frame_range: str) -> Tuple[float]:
    # Handle the format for frame_range
    if ':' in frame_range:
        start, stop = int(frame_range.split(':')[0]), int(frame_range.split(':')[1])
    else:
        start, stop = 0, int(frame_range)

    if input_bit_depth == 8:
        bytes_per_sample = 1
        read_type = np.uint8
    elif input_bit_depth in (9, 10, 11, 12, 13, 14, 15, 16):
        bytes_per_sample = 2
        read_type = np.uint16
    else:
        raise Exception(f"Bits per sample {input_bit_depth}, not valid, abort...")

    if chroma_format == "4:0:0":
        bytes_chroma = 0
    elif chroma_format == "4:2:0":
        bytes_chroma = int(bytes_per_sample * R * C * 0.5)
    elif chroma_format == "4:2:2":
        bytes_chroma = bytes_per_sample * R * C
    elif chroma_format == "4:4:4":
        bytes_chroma = bytes_per_sample * R * C * 2
    else:
        raise Exception(f"Unknown chroma format {chroma_format}, abort...")

    h = np.array([[1, 2, 1], [0, 0, 0], [-1, -2, -1]], np.float64) / 8

    is_rbg = '.rgb' in sequence_name

    SI_array, TI_array = np.zeros((stop - start, 1), np.float64), np.zeros((stop - start, 1), np.float64)
    Y_prev = np.zeros((R, C), np.int32)

    bytes_per_plane = R * C * bytes_per_sample

    with open(sequence_name, 'rb') as fh:
        i = 0
        for f in range(start, stop):
            if is_rbg:
                fh.seek(3 * f * bytes_per_plane)
                Red = np.reshape(np.frombuffer(fh.read(bytes_per_plane), dtype=read_type), (R, C)).astype(np.int32)
                Green = np.reshape(np.frombuffer(fh.read(bytes_per_plane), dtype=read_type), (R, C)).astype(np.int32)
                Blue = np.reshape(np.frombuffer(fh.read(bytes_per_plane), dtype=read_type), (R, C)).astype(np.int32)
                Y = bt709_conversion(Red, Green, Blue, input_bit_depth)
            else:
                index = f * (bytes_per_plane + bytes_chroma)
                fh.seek(index)
                Y = np.reshape(np.frombuffer(fh.read(bytes_per_plane), dtype=read_type), (R, C)).astype(np.int32)

            # Spatial information
            gx = dsp.convolve2d(Y, h, mode='valid')
            gy = dsp.convolve2d(Y, h.T, mode='valid')
            SI_array[i] = np.std(np.sqrt(np.square(gx) + np.square(gy)))

            # Temporal information
            if i:
                D = Y - Y_prev
                TI_array[i] = np.std(D)

            Y_prev = Y.copy()
            i += 1

    SI, TI = np.max(SI_array), np.max(TI_array)
    np.savetxt('si.txt', SI_array, fmt='%f', delimiter=',')
    np.savetxt('ti.txt', TI_array, fmt='%f', delimiter=',')

    return SI, TI


def bt709_conversion(Red: NDArray[(Any, Any), np.int32], Green: NDArray[(Any, Any), np.int32], Blue: NDArray[(Any, Any), np.int32], input_bit_depth: int) -> NDArray[(Any, Any), np.int32]:
    t = 0.2126 * Red + 0.7152 * Green + 0.0722 * Blue
    Y = (16 + 219 * t / 255 + 0.5).astype(np.int32)
    Y = np.clip(Y, 0, 1 << input_bit_depth)
    return Y
