'''
Full-reference image blurring metric computation, version 1.0
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

The blurmetric software computes the perceptual blur metric as
proposed in P. Marziliano, F. Dufaux, S. Winkler and T. Ebrahimi,
"Perceptual blur and ringing metrics: application to JPEG2000", Signal
Processing: Image Communication, vol. 19, no. 2, pp. 163-172, February
2004.
The metric works in full-reference modality and at the image level.
Each image is assumed as a 8-bit gray levels content.

Input parameters:
    - Y_ori            = Luminance component of the original image
    - Y_blur           = Luminance component of the blurred image

Output parameters:
    - metric_score     = Metric value at the frame level
    - width_array      = Matrix of size # x 1 whereas # denotes the
                         number of vertical edge image pixels. Each
                         matrix value contains the edge pixel width
    - image_edge_width = 3D array with the same height and width of the
                         original image which contains for each edge pixel
                         its start and end column associated with the
                         edge width
'''

import numpy as np
from nptyping import NDArray
from typing import Any, Tuple
import os
import sys


def blurmetric(Y_ori: NDArray[(Any, Any), np.int32], Y_blur: NDArray[(Any, Any), np.int32]) -> Tuple[float, NDArray[(Any), np.float64], NDArray[(Any, Any, 2), np.int32]]:
    R, C = Y_ori.shape[0], Y_ori.shape[1]

    # Edge map calculation
    path2edge_detector = os.path.dirname(os.path.realpath(__file__)) + '/../image-processing/edge-detection'
    sys.path.insert(1, path2edge_detector)
    from edgesobel import edgesobel
    edge_map = edgesobel(Y_ori, 'vertical')

    edge_idx = np.where(edge_map)

    width_array = np.zeros((len(edge_idx[0])), np.int32)

    image_edge_width = -1 * np.ones((R, C, 2), np.int32)

    i = 0

    for r_edge, c_edge in zip(edge_idx[0], edge_idx[1]):
        c_start, c_end = None, None

        # Check for left column
        if c_edge == 0:
            c_start = 0
        else:
            left_column = Y_blur[r_edge, :c_edge]
            d_left = np.diff(left_column[::-1])

            if Y_blur[r_edge, c_edge] > Y_blur[r_edge, c_edge - 1]:
                # Increasing trend toward the edge (/), look backwards
                # and find the local minum
                idx = np.where(d_left > 0)[0]
                if len(idx):
                    c_start = c_edge - idx[0] - 1
                else:
                    c_start = 0
            else:
                # Decreasing trend toward the edge (\), look backwards
                # and find the local maximum
                idx = np.where(d_left < 0)[0]
                if len(idx):
                    c_start = c_edge - idx[0] - 1
                else:
                    c_start = 0

        # Check for right column
        if c_edge == C - 1:
            c_end = C - 1
        else:
            right_column = Y_blur[r_edge, c_edge + 1:]
            d_right = np.diff(right_column)

            if Y_blur[r_edge, c_edge] < Y_blur[r_edge, c_edge + 1]:
                # Increasing trend toward the edge (/), look forward
                # and find the local maximum
                idx = np.where(d_right < 0)[0]
                if len(idx):
                    c_end = c_edge + idx[0] + 1
                else:
                    c_end = C - 1
            else:
                # Decreasing trend toward the edge (\), look forward
                # and find the local minimum
                idx = np.where(d_right > 0)[0]
                if len(idx):
                    c_end = c_edge + idx[0] + 1
                else:
                    c_end = C - 1

        width_array[i] = c_end - c_start
        image_edge_width[r_edge, c_edge, :] = [c_start, c_end]

        i += 1

    if len(np.where(width_array < 0)[0]):
        raise Exception("Problem with negative width!")

    metric_score = np.average(width_array)
    return metric_score, width_array, image_edge_width
