'''
h264qpel, version 1.0
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

The h264qpel software performs the quarter pixel upsampling as
specified in the H.264/AVC standard for the luma component of video signals.
The quarter pixel upsampling is performed in the following steps:
STEP 1: the values of the Y image are copied into the integer positions
        of the [R*4 C*4] Y4 image.
STEP 2: the values of the top, bottom, left and right edges are
        extended to their respective 4*4 areas
STEP 3: the values for the pixels at half positions and across
        integer positions are computed for the horizontal dimensions
STEP 4: the values for the pixels at half positions and across
        integer positions are computed for the vertical dimensions as
        well as the half positions between the half positions retrived
        previously
STEP 5: the quarter positions are computed according the standard
        specifications

The samples interpolated will refer to the following arrangement (see Figure 8 - 4)
of the H.264/AVC specification (https://www.itu.int/rec/T-REC-H.264-201906-I/en):

---------------------
| G | a | b | c | H |
---------------------
| d | e | f | g |   |
---------------------
| h | i | j | k | m |
---------------------
| n | p | q | r |   |
---------------------
| M |   | s |   | N |
---------------------

Parameters:
    - Input:
        Y  = a matrix of R x C dimensions containing the luma pixels
             of the image being upsampled
    - Output:
        Y4 = upsampled image given as output. This matrix has
             dimensions of R*4 + 4*4*2 x C*4 + 4*4*2, whereby the term
             4*4*2 accounts for the edge padding extension
    Note:
        Input and output values are assumed in the range [0 -255], so a
        clipping is performed accordingly.
'''

import numpy as np
from nptyping import NDArray
from typing import Any

def h264qpel(Y: NDArray[(Any, Any), np.int32]) -> NDArray[(Any, Any), np.int32]:
    PAD_FACTOR = 4
    R, C = Y.shape[0], Y.shape[1]
    R4 = (R + 2 * PAD_FACTOR) << 2
    C4 = (C + 2 * PAD_FACTOR) << 2
    Y4 = np.zeros((R4, C4), np.int32)

    ###############################################################################################
    # STEP 1: Replacing the integer position pixels (G, H, M, N)
    ###############################################################################################
    r_start, c_start = PAD_FACTOR << 2, PAD_FACTOR << 2
    r_end, c_end = R4 - r_start, C4 - c_start
    Y4[r_start:r_end:PAD_FACTOR, c_start:c_end:PAD_FACTOR] = Y

    ###############################################################################################
    # STEP 2: top, bottom, left and right edges padding (for out of boundary motion vectors)
    ###############################################################################################
    # 1) Top
    Y4[:16:PAD_FACTOR, c_start:c_end:PAD_FACTOR] = np.tile(Y4[r_start, c_start:c_end:PAD_FACTOR], (PAD_FACTOR, 1))

    # 2) Bottom
    Y4[r_end::PAD_FACTOR, c_start:c_end:PAD_FACTOR] = np.tile(Y4[r_end - 4, c_start:c_end:PAD_FACTOR], (PAD_FACTOR, 1))

    # 3) Left
    Y4[r_start:r_end:PAD_FACTOR, :c_start:PAD_FACTOR] = np.tile(Y4[r_start:r_end:PAD_FACTOR, c_start:c_start+1], (1, PAD_FACTOR))

    # 4) Right
    Y4[r_start:r_end:PAD_FACTOR, c_end::PAD_FACTOR] = np.tile(Y4[r_start:r_end:PAD_FACTOR, c_end - 4:c_end - 3], (1, PAD_FACTOR))

    # 5) Upper left corner
    Y4[:16:PAD_FACTOR, :16:PAD_FACTOR] = Y4[0, c_start]

    # 6) Upper right corner
    Y4[:16:PAD_FACTOR, c_end::PAD_FACTOR] = Y4[0, c_end - 4]

    # 7) Lower left corner
    Y4[r_end::PAD_FACTOR, :c_start - 1:PAD_FACTOR] = Y4[r_end, c_start]

    # 8) Lower right corner
    Y4[r_end::PAD_FACTOR, c_end::PAD_FACTOR] = Y4[r_end - 4, c_end - 4]

    ###############################################################################################
    # Step 3: half positions among integer positions and along the horizontal dimension (b, s)
    ###############################################################################################
    offset = np.array([-10, -6, -2, 2, 6, 10], np.int32)
    six_tap = np.array([1, -5, 20, 20, -5, 1], np.int32)

    Y4_temp = np.zeros((R4, C4), np.int32)
    c_range = np.array([[i for i in range(2, C4, PAD_FACTOR)]], np.int32)
    offset_matrix = np.tile(offset, (c_range.shape[1], 1))
    c_range_matrix = np.tile(c_range.T, (1, 6))
    c_selector = c_range_matrix + offset_matrix
    c_selector[np.where(c_selector < 0)] = 0
    c_selector[np.where(c_selector >= C4)] = C4 - 4

    i = 0
    for c in c_range[0]:
        Y4_temp[0:R4:PAD_FACTOR, c] = np.dot(Y4[0:R4:PAD_FACTOR, c_selector[i, :]], six_tap)
        Y4[0:R4:PAD_FACTOR, c] = (Y4_temp[0:R4:PAD_FACTOR, c] + 16) >> 5
        i += 1

    ###############################################################################################
    # STEP 4: half positions among integer positions along the vertical dimension and half pel
    # positions between half positions previously derived (h, m, j)
    ###############################################################################################
    r_range = np.array([[i for i in range(2, R4, PAD_FACTOR)]], np.int32)
    offset_matrix = np.tile(offset, (r_range.shape[1], 1))
    r_range_matrix = np.tile(r_range.T, (1, 6))
    r_selector = r_range_matrix + offset_matrix
    r_selector[np.where(r_selector < 0)] = 0
    r_selector[np.where(r_selector >= R4)] = R4 - 4

    i = 0
    for r in r_range[0]:
        Y4_temp[r, 0:C4:PAD_FACTOR] = np.dot(six_tap, Y4[r_selector[i, :], 0:C4:PAD_FACTOR])
        Y4[r, 0:C4:PAD_FACTOR] = (Y4_temp[r, 0:C4:PAD_FACTOR] + 16) >> 5
        temp = np.dot(six_tap, Y4_temp[r_selector[i, :], 2:C4:PAD_FACTOR])
        Y4[r, 2:C4:PAD_FACTOR] = (temp + 512) >> 10
        i += 1

    # Clip the values as per the JM's implementation
    Y4[np.where(Y4 < 0)] = 0
    Y4[np.where(Y4 > 255)] = 255

    ###############################################################################################
    # STEP 5: Quarter pixels positions (a, c, i, k)
    ###############################################################################################
    Y4[:R4:2, 1:C4 - 2:2] = (Y4[:R4:2, :C4 - 2:2] + Y4[:R4:2, 2:C4:2] + 1) >> 1

    # Column quarter pixels (d, n, f, q)
    Y4[1:R4 - 2:2, :C4:2] = (Y4[:R4 - 2:2, :C4:2] + Y4[2:R4:2, :C4:2] + 1) >> 1

    # Quarter pixels interpolated as /
    # Pixels starting from row 2 (e)
    Y4[1::PAD_FACTOR, 1::PAD_FACTOR] = (Y4[::PAD_FACTOR, 2::PAD_FACTOR] + Y4[2::PAD_FACTOR, ::PAD_FACTOR] + 1) >> 1

    # Pixels starting from row 4 (r)
    Y4[3:-1:PAD_FACTOR, 3:-1:PAD_FACTOR] = (Y4[4::PAD_FACTOR, 2:-2:PAD_FACTOR] + Y4[2:-2:PAD_FACTOR, 4::PAD_FACTOR] + 1) >> 1


    # Quarter pixels interpolated as \
    # Pixels starting from row 2 (g)
    Y4[1:-1:PAD_FACTOR, 3:-1:PAD_FACTOR] = (Y4[::PAD_FACTOR, 2:-2:PAD_FACTOR] + Y4[2::PAD_FACTOR, 4::PAD_FACTOR] + 1) >> 1

    # Pixels starting from row 4 (p)
    Y4[3:-1:PAD_FACTOR, 1:-1:PAD_FACTOR] = (Y4[2:-2:PAD_FACTOR, ::PAD_FACTOR] + Y4[4::PAD_FACTOR, 2::PAD_FACTOR] + 1) >> 1


    # Bottom row and rightmost column
    i = np.where(Y4[-1, 0:-1] == 0)
    b_row_minus1 = Y4[-2, :]
    Y4[-1, i] = b_row_minus1[i]

    i = np.where(Y4[:, -1] == 0)
    r_col_minus1 = Y4[:, -2]
    Y4[i, -1] = r_col_minus1[i]
    return Y4
