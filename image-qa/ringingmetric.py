'''
Full-reference image ringing metric computation, version 1.0
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

The ringingmetric software computes the perceptual ringing metric
as proposed in P. Marziliano, F. Dufaux, S. Winkler and T. Ebrahimi,
"Perceptual blur and ringing metrics: application to JPEG2000", Signal
Processing: Image Communication, vol. 19, no. 2, pp. 163-172, February
2004.
The metric works in full-reference modality and at the image level.
Each image is assumed as a 8-bit grey levels content.

Input parameters:
    - Y_ori            = Luma component of the original image
    - Y_ring           = Luma component of the image impaired with
                         ringing

Output parameters:
    - metric_score   = Image level ringing metric value
    - fixed_ringwidth  = Average extension of the ring impairment. If
                         the ring_width parameter is provided, this
                         output parameter is equal to ring_width.
                         Otherwise is the value computed by means of
                         the marziliano_blur software
    - ring_measure     = Measure of the ringing extension around an
                         image edge pixel (i.e. both for the right and
                         left sides).
    - Y_edge_ring      = Matrix of the same image sizes whereby each
                         pixel contains the average ringing extension
                         around an image pixel.
'''

from typing import Any, Tuple

import numpy as np
from blurmetric import blurmetric
from nptyping import NDArray


def ringingmetric(Y_ori: NDArray[(Any, Any), np.int32], Y_ring: NDArray[(Any, Any), np.int32]) -> Tuple[float, float, NDArray[(Any), np.float64], NDArray[(Any, Any, 2), np.int32]]:
    R, C = Y_ori.shape

    blur_metric_score, _, image_edge_width = blurmetric(Y_ori, Y_ring)
    fixed_ringwidth = int(blur_metric_score / 2 + 0.5)

    # Find the row and column coordinates for each edge
    edge_idx = np.where(image_edge_width[:, :, 0] != -1)

    # Compute the difference image
    D = Y_ring - Y_ori

    # Ring measure vector
    ring_measure = np.zeros((len(edge_idx[0]), 2), np.int32)  # [edge_number, left/right]

    # Output to display
    Y_edge_ring = np.zeros((R, C), np.float64)

    # Loop over each edge pixel
    i = 0
    for r_edge, c_edge in zip(edge_idx[0], edge_idx[1]):
        c_start = image_edge_width[r_edge, c_edge, 0]
        c_end = image_edge_width[r_edge, c_edge, 1]

        # Left ringing area
        c_left_ringing_start = max(0, c_edge - fixed_ringwidth)
        c_left_ringing_end = c_start
        left_ringwidth = np.abs(c_left_ringing_end - c_left_ringing_start)

        # Rigth ringing area
        c_right_ringing_start = c_end
        c_right_ringing_end = min(C - 1, c_edge + fixed_ringwidth)
        right_ringwidth = np.abs(c_right_ringing_end - c_right_ringing_start)

        # Make the ringing lobe vectors
        if c_left_ringing_start <= c_left_ringing_end:
            selector = slice(c_left_ringing_start, c_left_ringing_end + 1)
        else:
            selector = slice(c_left_ringing_start, max(0, c_left_ringing_end - 1), -1)

        left_vector = D[r_edge, selector]

        if c_right_ringing_start <= c_right_ringing_end:
            selector = slice(c_right_ringing_start, c_right_ringing_end + 1)
        else:
            selector = slice(c_right_ringing_start, max(0, c_right_ringing_end - 1), -1)

        right_vector = D[r_edge, selector]

        # Left ring measure
        ring_measure[i, 0] = left_ringwidth * np.abs(np.max(left_vector) - np.min(left_vector))
        ring_measure[i, 1] = right_ringwidth * np.abs(np.max(right_vector) - np.min(right_vector))

        Y_edge_ring[r_edge, c_edge] = (ring_measure[i, 0] + ring_measure[i, 1]) / 2

        i += 1

    # Metric computation sanity check
    if len(np.where(ring_measure < 0)[0]):
        raise Exception("Problem with negative ringing measure!")

    # Final metric computation
    ringing_metric_score = np.mean(ring_measure)

    return ringing_metric_score, fixed_ringwidth, ring_measure, Y_edge_ring
