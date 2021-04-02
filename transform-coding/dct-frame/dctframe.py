'''
dctframe, version 1.0
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

The dctframe software performs the computation of the Type II DCT over a
nonoverlapped grid of disjoint [block_dim x block_dim] blocks
Parameters:
    - Input:
        x         = matrix representing the input data (e.g. luminance
                    component of a given image)
        block_dim = block dimension
    - Ouput:
        X         = matrix containing the DCT transform of x
Note:
    If the image height and/or width are not a multiple of block_dim, the height and/or
    width of the output will be cropped accordingly.
    Although no check is performed, power of 2 values should be used for block_dim.
    Indeed, it is an immensely stupid choice to use block_dim values which are not
    a power of two.
'''

from typing import Any

import numpy as np
from nptyping import NDArray


def dctframe(x: NDArray[(Any, Any), np.int], block_dim: int) -> NDArray[(Any, Any), np.float64]:
    R, C = x.shape
    R = (R // block_dim) * block_dim
    C = (C // block_dim) * block_dim

    X = np.zeros((R, C), np.float64)

    T = np.zeros((block_dim, block_dim), np.float64)

    j, i = np.meshgrid(np.arange(block_dim), np.arange(block_dim))

    T[0, :] = 1 / np.sqrt(block_dim)

    T[1::, :] = np.sqrt(2 / block_dim) * np.cos(np.pi * (np.multiply(i[1::, :], j[1::, :] + 0.5) / block_dim))

    Rb, Cb = R // block_dim, C // block_dim

    for r in range(Rb):
        range_r = slice(r * block_dim, (r + 1) * block_dim)
        for c in range(Cb):
            range_c = slice(c * block_dim, (c + 1) * block_dim)
            block = x[range_r, range_c].astype(np.float64)
            X[range_r, range_c] = np.matmul(T, np.matmul(block, T.T))

    return X
