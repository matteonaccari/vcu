function [metric_score width_array image_edge_width] = blurmetric(Y_ori, Y_blur)
%   Full-reference image blurring metric computation, version 1.0
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
%   The blurmetric software computes the perceptual blur metric as
%   proposed in P. Marziliano, F. Dufaux, S. Winkler and T. Ebrahimi,
%   "Perceptual blur and ringing metrics: application to JPEG2000", Signal
%   Processing: Image Communication, vol. 19, no. 2, pp. 163-172, February
%   2004.
%   The metric works in full-reference modality and at the image level.
%   Each image is assumed as a 8-bit gray levels content.
%
%   Input parameters:
%       - Y_ori            = Luminance component of the original image
%       - Y_blur           = Luminance component of the blurred image
%
%   Output parameters:
%       - metric_score     = Metric value at the frame level
%       - width_array      = Matrix of size # x 1 whereas # denotes the
%                            number of vertical edge image pixels. Each
%                            matrix value contains the edge pixel width
%       - image_edge_width = 3D array with the same height and width of the
%                            original image which contains for each edge pixel
%                            its start and end column associated with the
%                            edge width. For non edge pixels a conventional value
%                            of plus infinity is set

%   Get the image sizes
[R C] = size(Y_ori);

Y_ori = uint8(Y_ori);

%   Step 1: Sobel edge detection
edge_map = edge(Y_ori, 'sobel', 'vertical');

%   Step 2: Find the starting and ending position for each edge
[col row] = find(edge_map');
width_array = zeros(size(row));

image_edge_width = inf * ones(R, C, 2);

for i = 1:length(row)

    r_edge = row(i);
    c_edge = col(i);

    c_start = inf;
    c_end   = inf;

    %   Check for left side
    if c_edge == 1
        c_start = 1;
    else
        left_side = Y_blur(r_edge, 1:c_edge - 1);
        d_left = diff(left_side(end:-1:1));
        if Y_blur(r_edge, c_edge) > Y_blur(r_edge, c_edge - 1)
            %   Increasing trend towards the edge (/), look backwards
            %   and find the local minum
            idx = find(d_left > 0);
            if ~isempty(idx)
                c_start = c_edge - idx(1);
            else
                c_start = 1;
            end
        else
            %   Decreasing trend towards the edge (\), look backwards
            %   and find the local maximum
            idx = find(d_left < 0);
            if ~isempty(idx)
                c_start = c_edge - idx(1);
            else
                c_start = 1;
            end
        end
    end

    %   Check for right side
    if c_edge == C
        c_end = C;
    else
        right_side = Y_blur(r_edge, c_edge + 1:end);
        d_right = diff(right_side);
        if Y_blur(r_edge, c_edge) < Y_blur(r_edge, c_edge + 1)
            %   Increasing trend towards the edge (/), look forward
            %   and find the local maximum
            idx = find(d_right < 0);
            if ~isempty(idx)
                c_end = c_edge + idx(1);
            else
                c_end = C;
            end
        else
            %   Decreasing trend towards the edge (\), look forward
            %   and find the local minimum
            idx = find(d_right > 0);
            if ~isempty(idx)
                c_end = c_edge + idx(1);
            else
                c_end = C;
            end
        end
    end

    width_array(i) = c_end - c_start;

    image_edge_width(r_edge, c_edge, :) = [c_start, c_end];

end

%   Metric computation sanity check
if ~isempty(find(width_array < 0, 1))
    error('Problem with negative width!\n');
end

%   Final metric computation
metric_score = mean(width_array);