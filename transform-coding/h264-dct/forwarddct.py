'''
Forward H.264/AVC 4x4 integer DCT and associated quantisation, version 1.0
Copyright(c) 2021 Matteo Naccari (matteo.naccari@gmail.com)
All Rights Reserved.

email: matteo.naccari@gmail.com | matteo.naccari@polimi.it | matteo.naccari@gmail.com

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

The forwarddct software compute the forward integer DCT approximation as specified in the
H.264/AVC standard. The quantisation process is also performed given that this is where the
orthornomality of the integer approximation is adjusted. For the more details, the reader is
referred here:
https://www.vcodex.com/h264avc-4x4-transform-and-quantization/
'''

from typing import Any

import numpy as np
from nptyping import NDArray

T = np.array([[1, 1, 1, 1],
              [2, 1, -1, -2],
              [1, -1, -1, 1],
              [1, -2, 2, -1]], np.int32)

offset = np.array([10912, 21824, 43648, 87296, 174592, 349184, 698368, 1396736, 2793472, 5586944, 11173888, 22347776], np.int32)

postscaling_factor = np.array([[[13107, 8066, 13107, 8066], [8066, 5243, 8066, 5243], [13107, 8066, 13107, 8066], [8066, 5243, 8066, 5243]],
                               [[11916, 7490, 11916, 7490], [7490, 4660, 7490, 4660], [11916, 7490, 11916, 7490], [7490, 4660, 7490, 4660]],
                               [[10082, 6554, 10082, 6554], [6554, 4194, 6554, 4194], [10082, 6554, 10082, 6554], [6554, 4194, 6554, 4194]],
                               [[9362, 5825, 9362, 5825], [5825, 3647, 5825, 3647], [9362, 5825, 9362, 5825], [5825, 3647, 5825, 3647]],
                               [[8192, 5243, 8192, 5243], [5243, 3355, 5243, 3355], [8192, 5243, 8192, 5243], [5243, 3355, 5243, 3355]],
                               [[7282, 4559, 7282, 4559], [4559, 2893, 4559, 2893], [7282, 4559, 7282, 4559], [4559, 2893, 4559, 2893]]], np.int32)


def forwarddct(frame: NDArray[(Any, Any), np.int32], QP: int) -> NDArray[(Any, Any), np.int32]:
    rows, cols = frame.shape[0], frame.shape[1]

    dct_frame = np.zeros((rows, cols), np.int32)
    ps = postscaling_factor[QP % 6]
    rounding = offset[QP // 6]
    qbits = 15 + QP // 6

    for r in range(0, rows, 4):
        r_slice = slice(r, r + 4)
        for c in range(0, cols, 4):
            c_slice = slice(c, c + 4)
            block = frame[r_slice, c_slice]

            # Forward tranformation T * block * T'
            t_block = np.matmul(T, np.matmul(block, T.T))

            # Post scaling operation
            s = np.sign(t_block)
            s_block = (np.multiply(np.abs(t_block), ps) + rounding) >> qbits
            q_block = np.multiply(s, s_block)

            dct_frame[r_slice, c_slice] = q_block

    return dct_frame
