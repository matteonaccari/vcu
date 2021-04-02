'''
Image edge calculation using the Sobel operator (edgesobel), version 1.0
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

The edgesobel software determines the pixels associated with the image edges using
the Sobel operator and automatic thresholding. The software mimics the processing of
the edge function in Matlab and GNU Octave when it is called as:
    edge(I, 'sobel', 'direction')

Parameters:
    - Input:
        image     = 2D array containing the image pixels associated with a given
                    colour component (e.g. luma).
                    images are assumed as 8-bit grey level content
        direction = dimension along which the gradient is computed via the Sobel operator.
                    Valid values are: horizontal, vertical, both (default)

    - Output:
        edge_map  = 2D array of boolean elements, each indicating whether the pixel is
                    associated with an image edge.
'''

from typing import Any

import numpy as np
import scipy.signal as dsp
from nptyping import Bool, NDArray


def edgesobel(image: NDArray[(Any, Any), np.int32], direction: str = 'both') -> NDArray[(Any, Any), Bool]:
    if direction not in ('horizontal', 'vertical', 'both'):
        raise Exception(f'Direction {direction} not recognised')

    if len(image.shape) > 2:
        raise Exception("Only 2D arrays can be processed")

    rows, cols = image.shape[0], image.shape[1]
    h = np.array([[1, 2, 1],
                  [0, 0, 0],
                  [-1, -2, -1]], np.float64)
    h /= np.sum(np.abs(h))
    pad_image = np.pad(image, ((1, 1), (1, 1)), 'edge')

    # Rescale the input image into the range [0, 1]
    pad_image = (pad_image - np.min(pad_image)) / np.max(pad_image)

    if direction == 'horizontal':
        grad2 = np.square(dsp.convolve2d(pad_image, h, 'valid'))
    elif direction == 'vertical':
        grad2 = np.square(dsp.convolve2d(pad_image, h.T, 'valid'))
    else:
        grad2 = np.square(dsp.convolve2d(pad_image, h, 'valid')) + np.square(dsp.convolve2d(pad_image, h.T, 'valid'))

    threshold2 = 4 * np.average(grad2)

    idx = np.where(grad2 <= threshold2)
    grad2[idx] = 0

    # Do the thinning
    sx = (grad2 > np.concatenate((np.zeros((rows, 1)), grad2[:, :-1]), axis=1)) & (grad2 > np.concatenate((grad2[:, 1:], np.zeros((rows, 1))), axis=1))
    sy = (grad2 > np.concatenate((np.zeros((1, cols)), grad2[:-1, :]), axis=0)) & (grad2 > np.concatenate((grad2[1:, :], np.zeros((1, cols))), axis=0))
    edge_map = sx | sy

    return edge_map
