'''
Ringing impairment addition to images (addringing), version 1.0
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

The image_ringing software adds the ringing impairment (also denoted as
edge busyness) to a given image passed as input parameter. The ringing
impairment is added according to what specified in ITU-T, "Principles
of a reference impairment system for video", Recommendation ITU-T
P.930, August 1996.

Parameters:
    - Input:
        Y_ori            = Original image content being impaired. The
                           images are assumed as 8-bit grey level content
        echo_amplitude   = Amplitude of the echo added to create the
                           displacement halo which turns into the
                           ringing impairment. Values must be in the
                           integer range of [-30, -1] inclusive.
       echo_displacement = Displacement value for the echo being added
                           to create the ringing impairment. Valid values are:
                           [0.375, 0.5, 0.75]

    - Output:
        Y_ring           = Original image impaired with ringing
'''

from typing import Any

import numpy as np
import scipy.signal as dsp
from nptyping import NDArray


def addringing(Y: NDArray[(Any, Any), np.int32], echo_amplitude: int, echo_displacement: float, bit_depth: int = 8) -> NDArray[(Any, Any), np.int32]:
    # Half filter size
    half_size = 6

    max_value = (1 << bit_depth) - 1

    if not -30 <= echo_amplitude and echo_amplitude <= -1:
        raise Exception("Echo amplitude must be in the range [-30, -1] inclusive")

    index_echo_disp = {0.375: 0, 0.5: 1, 0.75: 2}

    all_taps = np.array([[0, 0, 0, echo_amplitude, 0, 0, 175, 0, 0, echo_amplitude, 0, 0, 0],
                         [0, 0, echo_amplitude, 0, 0, 0, 175, 0, 0, 0, echo_amplitude, 0, 0],
                         [echo_amplitude, 0, 0, 0, 0, 0, 175, 0, 0, 0, 0, 0, echo_amplitude]], np.float64)

    f_window = np.zeros(((half_size << 1) + 1, (half_size << 1) + 1), np.float64)

    f_window[6, :] = all_taps[index_echo_disp[echo_displacement]]

    f_window /= np.sum(f_window)

    # Horizontal filtering
    Y_hor_filt = dsp.convolve2d(Y.astype(np.float64), f_window, "same")

    # Vertical filtering
    Y_ring = dsp.convolve2d(Y_hor_filt, f_window.T, "same")

    # Final rounding and clipping
    Y_ring = np.round(Y_ring)
    Y_ring = np.clip(Y_ring, 0, max_value)

    return Y_ring.astype(np.int32)
