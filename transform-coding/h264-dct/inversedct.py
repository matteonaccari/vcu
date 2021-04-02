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

rescaling_factor = np.array([[[10, 13, 10, 13], [13, 16, 13, 16], [10, 13, 10, 13], [13, 16, 13, 16]],
                             [[11, 14, 11, 14], [14, 18, 14, 18], [11, 14, 11, 14], [14, 18, 14, 18]],
                             [[13, 16, 13, 16], [16, 20, 16, 20], [13, 16, 13, 16], [16, 20, 16, 20]],
                             [[14, 18, 14, 18], [18, 23, 18, 23], [14, 18, 14, 18], [18, 23, 18, 23]],
                             [[16, 20, 16, 20], [20, 25, 20, 25], [16, 20, 16, 20], [20, 25, 20, 25]],
                             [[18, 23, 18, 23], [23, 29, 23, 29], [18, 23, 18, 23], [23, 29, 23, 29]]], np.int32)


def inversedct(coefficients: NDArray[(Any, Any), np.int32], QP: int) -> NDArray[(Any, Any), np.int32]:
    rows, cols = coefficients.shape
    rec_frame = np.zeros((rows, cols), np.int32)
    rf = rescaling_factor[QP % 6]
    qbits = QP // 6

    for r in range(0, rows, 4):
        r_slice = slice(r, r + 4)
        for c in range(0, cols, 4):
            c_slice = slice(c, c + 4)
            block = coefficients[r_slice, c_slice]

            # Rescaling
            r_block = np.multiply(block, rf) << qbits

            # Inverse transformation: this could also be implemented as matrix multiplication
            # but it may introduce rounding errors, leading to discrepancies with the Matlab/GNU Octave
            # implementation
            rec_frame[r_slice, c_slice] = inverse_core_transform(r_block)

    return rec_frame


def inverse_core_transform(block: NDArray[(4, 4), np.int32]) -> NDArray[(4, 4), np.int32]:
    output_block = np.zeros((4, 4), np.int32)
    # horizontal
    for i in range(4):
        s02 = (block[i][0] + block[i][2])
        d02 = (block[i][0] - block[i][2])
        s13 = block[i][1] + (block[i][3] >> 1)
        d13 = (block[i][1] >> 1) - block[i][3]

        output_block[i][0] = (s02 + s13)
        output_block[i][1] = (d02 + d13)
        output_block[i][2] = (d02 - d13)
        output_block[i][3] = (s02 - s13)

    # vertical
    for i in range(4):
        s02 = (output_block[0][i] + output_block[2][i])
        d02 = (output_block[0][i] - output_block[2][i])
        s13 = output_block[1][i] + (output_block[3][i] >> 1)
        d13 = (output_block[1][i] >> 1) - output_block[3][i]

        output_block[0][i] = ((s02 + s13 + 32) >> 6)
        output_block[1][i] = ((d02 + d13 + 32) >> 6)
        output_block[2][i] = ((d02 - d13 + 32) >> 6)
        output_block[3][i] = ((s02 - s13 + 32) >> 6)

    return output_block
