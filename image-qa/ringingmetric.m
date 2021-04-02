function [metric_score fixed_ringwidth ring_measure Y_edge_ring] = ringingmetric(Y_ori, Y_ring)
%   Full-reference image ringing metric computation, version 1.0
%   Copyright(c) 2021 Matteo Naccari
%   All Rights Reserved.
%
%   email: matteo.naccari@gmail.com | matteo.naccari@polimi.it | matteo.naccari@lx.it.pt
%
%   The copyright in this software is being made available under the BSD
%   License, included below. This software may be subject to other third party
%   and contributor rights, including patent rights, and no such rights are
%   granted under this license.
%   Redistribution and use in source and binary forms, with or without
%   modification, are permitted provided that the following conditions are met:
%    * Redistributions of source code must retain the above copyright notice,
%      this list of conditions and the following disclaimer.
%    * Redistributions in binary form must reproduce the above copyright notice,
%      this list of conditions and the following disclaimer in the documentation
%      and/or other materials provided with the distribution.
%    * Neither the name of the author may be used to endorse or promote products derived
%      from this software without specific prior written permission.

%   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
%   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
%   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
%   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
%   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
%   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
%   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
%   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
%   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
%   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
%   THE POSSIBILITY OF SUCH DAMAGE.
%
%   The ringingmetric software computes the perceptual ringing metric
%   as proposed in P. Marziliano, F. Dufaux, S. Winkler and T. Ebrahimi,
%   "Perceptual blur and ringing metrics: application to JPEG2000", Signal
%   Processing: Image Communication, vol. 19, no. 2, pp. 163-172, February
%   2004.
%   The metric works in full-reference modality and at the image level.
%   Each image is assumed as a 8-bit grey levels content.
%
%   Input parameters:
%       - Y_ori            = Luma component of the original image
%       - Y_ring           = Luma component of the image impaired with
%                            ringing
%
%   Output parameters:
%       - metric_score   = Image level ringing metric value
%       - fixed_ringwidth  = Average extension of the ring impairment. If
%                            the ring_width parameter is provided, this
%                            output parameter is equal to ring_width.
%                            Otherwise is the value computed by means of
%                            the marziliano_blur software
%       - ring_measure     = Measure of the ringing extension around an
%                            image edge pixel (i.e. both for the right and
%                            left sides).
%       - Y_edge_ring      = Matrix of the same image sizes whereby each
%                            pixel contains the average ringing extension
%                            around an image pixel.

Y_ori  = uint8(Y_ori);

[R C] = size(Y_ori);

[blur_metric, ~, image_edge_width] = blurmetric(Y_ori, Y_ring);
fixed_ringwidth = round(blur_metric/2);

%   Find the row and column coordinates for each edge
[col row] = find(image_edge_width(:, :, 1)' != inf);

%   Compute the difference image
D = double(Y_ring) - double(Y_ori);

%   Ring measure vector
ring_measure = zeros(length(row), 2); % [edge_number, left/right]

%   Output to display
Y_edge_ring = zeros(size(Y_ori));

%   Loop over each edge pixel
for i = 1:length(row)

    r_edge = row(i);
    c_edge = col(i);

    c_start = image_edge_width(r_edge, c_edge, 1);
    c_end = image_edge_width(r_edge, c_edge, 2);

    %   Left ringing area
    c_left_ringing_start = max(1, c_edge - fixed_ringwidth);
    c_left_ringing_end   = c_start;
    left_ringwidth       = abs(c_left_ringing_end - c_left_ringing_start);

    %   Rigth ringing area
    c_right_ringing_start = c_end;
    c_right_ringing_end   = min(C, c_edge + fixed_ringwidth);
    right_ringwidth       = abs(c_right_ringing_end - c_right_ringing_start);

    %   Make the ringing lobe vectors
    if c_left_ringing_start <= c_left_ringing_end
        increment = 1;
    else
        increment = -1;
    end
    left_vector  = D(r_edge, c_left_ringing_start:increment:c_left_ringing_end);

    if c_right_ringing_start <= c_right_ringing_end
        increment = 1;
    else
        increment = -1;
    end
    right_vector = D(r_edge, c_right_ringing_start:increment:c_right_ringing_end);

    %   Left ring measure
    ring_measure(i, 1) = left_ringwidth*abs(max(left_vector) - min(left_vector));
    ring_measure(i, 2) = right_ringwidth*abs(max(right_vector) - min(right_vector));

    Y_edge_ring(r_edge, c_edge) = (ring_measure(i, 1) + ring_measure(i, 2))/2;

end

%   Metric computation sanity check
if ~isempty(find(ring_measure(:) < 0, 1))
    error('Problem with negative ringing measure!\n');
end

%   Final metric computation
metric_score = mean(ring_measure(:));
