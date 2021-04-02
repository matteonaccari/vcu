'''
No-reference Generalized Block Impairment Metric (GBIM), version 1.0
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
SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

The gbim software computes the perceptual blockiness metric
proposed in H. R. Wu and M. Yuen, "A generalized block-edge impairment
metric for video coding", IEEE Signal Process. Lett., vol. 4, no. 11,
pp. 317-320, Nov. 1997.
The metric works in no-reference modality and it may be parametrized
with respect to the square image block dimension.
Each image is assumed as a 8-bit grey levels content.

Input parameters:
    - image            = Luminance component for the image whose the GBIM
                         is being computed
    - B                = Block size to be considered in the GBIM
                         computation

Output parameters:
    - MGBIM            = Image level metric value
'''

from typing import Any

import numpy as np
from nptyping import NDArray


def gbim(image: NDArray[(Any, Any), np.int32], B: int) -> float:
    R, C = np.shape(image)
    RB, CB = R // B, C // B
    xi = 81
    _lambda = np.log(1 + np.sqrt(255 - xi)) / np.log(1 + np.sqrt(xi))

    # Dimensions check
    if R % B:
        raise Exception(f"Number of rows {R} is not a multiple of block size {B}")

    if C % B:
        raise Exception(f"Number of columns {C} is not a multiple of block size {B}")

    ###########################################################################
    # Horizontal blockiness
    ###########################################################################
    # Compute the Dcf matrix as in equation (1)
    right_c = slice(B - 1, C - B + 1, B)
    left_c = slice(B, C - B + 1, B)

    image_col_right = image[:, right_c].astype(np.int32)
    image_col_left = image[:, left_c].astype(np.int32)

    Dcf = image_col_right - image_col_left

    # Compute the mu(i,j) and the sigma(i,j) terms
    mu = np.zeros((R, CB - 1), np.float64)
    sigma = np.zeros((R, CB - 1), np.float64)
    for c in range(CB - 1):
        left_col = image[:, slice(c * B, (c + 1) * B)]
        right_col = image[:, slice((c + 1) * B, (c + 2) * B)]
        mu_left = np.mean(left_col, axis=1)
        mu_right = np.mean(right_col, axis=1)
        mu[:, c] = (mu_left + mu_right) / 2

        sigma_left = np.std(left_col, axis=1)
        sigma_right = np.std(right_col, axis=1)

        sigma[:, c] = (sigma_left + sigma_right) / 2

    # Compute the intermediate terms for the weight w
    index_low = np.where(mu <= xi)
    index_up = np.where(mu > xi)

    # arg1 = 1 + sqrt(mu)/(1 + sigma). For mu(i, j) <= xi
    arg1 = 1 + np.divide(np.sqrt(mu), (1 + sigma))

    # arg2 = 1 + sqrt(255 - mu)/(1 + sigma). For mu(i, j) > xi
    arg2 = 1 + np.divide(np.sqrt(255 - mu), (1 + sigma))

    # low_cond = lambda*log(arg1)
    low_cond = _lambda * np.log(arg1)

    # up_cond = log(arg2)
    up_cond = np.log(arg2)

    # Compute the term w as in equation (3)
    w = np.zeros((R, CB - 1), np.float64)
    w[index_low] = low_cond[index_low]
    w[index_up] = up_cond[index_up]

    # Compute the Mh matrix as in equation (1)
    temp = np.multiply(w, Dcf)
    Mh = np.sqrt(np.sum(np.square(temp)))

    # Compute the matrix E as in equation (5)
    S = np.zeros((B - 1))
    for n in range(B - 1):
        current = image[:, B + n::B]
        next = image[:, B + n + 1::B]
        D = current - next
        temp = np.multiply(w, D)
        S[n] = np.sqrt(np.sum(np.square(temp)))

    E = np.mean(S)

    # Compute MhGBIM
    MhGBIM = Mh / E

    ###########################################################################
    # Vertical blockiness
    ###########################################################################
    # Compute the vertical Dcf as in equation (1)
    top_r = slice(B - 1, R - B + 1, B)
    bot_r = slice(B, R - B + 1, B)

    image_row_top = image[top_r, :]
    image_row_bot = image[bot_r, :]

    Dcf = image_row_top - image_row_bot

    # Compute the mu(i,j) and the sigma(i,j) terms
    mu = np.zeros((RB - 1, C))
    sigma = np.zeros((RB - 1, C))
    for r in range(RB - 1):
        top_row = image[r * B:(r + 1) * B, :]
        bot_row = image[(r + 1) * B:(r + 2) * B, :]
        mu_top = np.mean(top_row, axis=0)
        mu_bot = np.mean(bot_row, axis=0)
        mu[r, :] = (mu_top + mu_bot) / 2

        sigma_top = np.std(top_row, axis=0)
        sigma_bot = np.std(bot_row, axis=0)

        sigma[r, :] = (sigma_top + sigma_bot) / 2

    # Compute the intermediate terms for the weight w
    index_low = np.where(mu <= xi)
    index_up = np.where(mu > xi)

    # arg1 = 1 + sqrt(mu)/(1 + sigma). For mu(i, j) <= xi
    arg1 = 1 + np.divide(np.sqrt(mu), (1 + sigma))

    # arg2 = 1 + sqrt(255 - mu)/(1 + sigma). for mu(i, j) > xi
    arg2 = 1 + np.divide(np.sqrt(255 - mu), (1 + sigma))

    # low_cond = lambda*log(arg1)
    low_cond = _lambda * np.log(arg1)

    # up_cond = log(arg2)
    up_cond = np.log(arg2)

    # Compute the weight w as in equation (3)
    w = np.zeros((RB - 1, C))
    w[index_low] = low_cond[index_low]
    w[index_up] = up_cond[index_up]

    # Compute the Mv matrix as in equation (2)
    temp = np.multiply(w, Dcf)
    Mv = np.sqrt(np.sum(np.square(temp)))

    # Compute the matrix E as in equation (5)
    S = np.zeros((B - 1))
    for n in range(B - 1):
        current = image[B + n::B, :]
        next = image[B + n + 1::B, :]
        D = current - next
        temp = np.multiply(w, D)
        S[n] = np.sqrt(np.sum(np.square(temp)))

    E = np.mean(S)

    # Compute the MvGBIM
    MvGBIM = Mv / E

    # Compute the final MGBIM at the frame level
    MGBIM = (MhGBIM + MvGBIM) / 2

    return MGBIM
