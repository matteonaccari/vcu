function Y4 = h264qpel(Y)
%   h264qpel, version 1.0
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
%   The h264qpel software performs the quarter pixel upsampling as
%   specified in the H.264/AVC standard for the luma component of video signals.
%   The quarter pixel upsampling is performed in the following steps:
%   STEP 1: the values of the Y image are copied into the integer positions
%           of the [R*4 C*4] Y4 image.
%   STEP 2: the values of the top, bottom, left and right edges are
%           extended to their respective 4*4 areas
%   STEP 3: the values for the pixels at half positions and across
%           integer positions are computed for the horizontal dimensions
%   STEP 4: the values for the pixels at half positions and across
%           integer positions are computed for the vertical dimensions as
%           well as the half positions between the half positions retrived
%           previously
%   STEP 5: the quarter positions are computed according the standard
%           specifications
%
%   The samples interpolated will refer to the following arrangement (see Figure 8 - 4)
%   of the H.264/AVC specification (https://www.itu.int/rec/T-REC-H.264-201906-I/en):
%
%   ---------------------
%   | G | a | b | c | H |
%   ---------------------
%   | d | e | f | g |   |
%   ---------------------
%   | h | i | j | k | m |
%   ---------------------
%   | n | p | q | r |   |
%   ---------------------
%   | M |   | s |   | N |
%   ---------------------
%
%   Parameters:
%       - Input:
%           Y  = a matrix of R x C dimensions containing the luma pixels
%                of the image being upsampled
%       - Output:
%           Y4 = upsampled image given as output. This matrix has
%                dimensions of R*4 + 4*4*2 x C*4 + 4*4*2, whereby the term
%                4*4*2 accounts for the edge padding extension
%       Note:
%           Input and output values are assumed in the range [0 -255], so a
%           clipping is performed accordingly.

PAD_FACTOR = 4;

[R C] = size(Y);

R4 = R*4 + 2*PAD_FACTOR*4;
C4 = C*4 + 2*PAD_FACTOR*4;

Y4 = zeros(R4, C4);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   STEP 1: Replacing the integer position pixels (G, H, M, N)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

r_start = PAD_FACTOR*4 + 1;
c_start = PAD_FACTOR*4 + 1;
r_end   = R4 - r_start + 1;
c_end   = C4 - c_start + 1;

Y4(r_start:PAD_FACTOR:r_end, c_start:PAD_FACTOR:c_end) = Y;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   STEP 2: top, bottom, left and right edges padding (for out of boundary
%   motion vectors)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   1) Top
Y4(1:PAD_FACTOR:16, c_start:PAD_FACTOR:c_end) = ...
    repmat(Y4(r_start, c_start:PAD_FACTOR:c_end), PAD_FACTOR, 1);

%   2) Bottom
Y4(r_end + 1:PAD_FACTOR:end, c_start:PAD_FACTOR:c_end) = ...
    repmat(Y4(r_end - 3, c_start:PAD_FACTOR:c_end), PAD_FACTOR, 1);

%   3) Left
Y4(r_start:PAD_FACTOR:r_end, 1:PAD_FACTOR:c_start - 1) = ...
    repmat(Y4(r_start:PAD_FACTOR:r_end, c_start), 1, PAD_FACTOR);

%   4) Right
Y4(r_start:PAD_FACTOR:r_end, c_end + 1:PAD_FACTOR:end) = ...
    repmat(Y4(r_start:PAD_FACTOR:r_end, c_end - 3), 1, PAD_FACTOR);

%   5) Upper left corner
Y4(1:PAD_FACTOR:16, 1:PAD_FACTOR:16) = Y4(1, c_start);

%   6) Upper right corner
Y4(1:PAD_FACTOR:16, c_end + 1:PAD_FACTOR:end) = Y4(1, c_end - 3);

%   7) Lower left corner
Y4(r_end + 1:PAD_FACTOR:end, 1:PAD_FACTOR:c_start-1) = Y4(r_end + 1, c_start);

%   8) Lower right corner
Y4(r_end + 1:PAD_FACTOR:end, c_end + 1:PAD_FACTOR:end) = ...
    Y4(r_end - 3, c_end - 3);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   STEP 3: half positions among integer positions and along the horizontal
%   dimension (b, s)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
offset = [-10:PAD_FACTOR:10];
six_tap = [1, -5, 20, 20, -5, 1];
six_tap_matrix = repmat(six_tap, R + PAD_FACTOR*2, 1);

Y4_temp = zeros(R4, C4);

range_c = 3:PAD_FACTOR:C4;
range_r = 1:PAD_FACTOR:R4;

offset_matrix = repmat(offset, length(range_c), 1);
range_c_matrix = repmat(range_c', 1, 6);

col_pred = range_c_matrix + offset_matrix;
col_pred(col_pred < 0) = 1;
col_pred(col_pred > C4) = C4 - 3;

i = 1;

for c = range_c
    Y4_temp(range_r, c) = sum((six_tap_matrix.*Y4(range_r, col_pred(i, :)))');
    Y4(range_r, c) = floor((Y4_temp(range_r, c) + 16)/32);
    i = i + 1;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   STEP 4: half positions among integer positions along the vertical
%   dimension and half positions between half positions previously derived (h, m, j)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
six_tap_matrix = repmat(six_tap', 1, C + PAD_FACTOR*2);
range_r = 3:PAD_FACTOR:R4;
range_c = 1:PAD_FACTOR:C4;
range_c_half =  3:PAD_FACTOR:C4;

offset_matrix = repmat(offset, length(range_r), 1);
range_r_matrix = repmat(range_r', 1, 6);

row_pred = range_r_matrix + offset_matrix;
row_pred(row_pred < 0) = 1;
row_pred(row_pred > R4) = R4 - 3;

i = 1;

for r = range_r
    Y4_temp(r, range_c) = sum(six_tap_matrix.*Y4(row_pred(i, :), range_c));
    Y4(r, range_c) = floor((Y4_temp(r, range_c) + 16)/32);
    %   Row half pixels between half pixels derived previously
    temp = sum(six_tap_matrix.*Y4_temp(row_pred(i, :), range_c_half));
    Y4(r, range_c_half) = floor((temp + 512)/1024);
    i = i + 1;
end

% Clip the values as per the JM's implementation
Y4(Y4 < 0) = 0;
Y4(Y4 > 255) = 255;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   STEP 5: Quarter pixels positions (a, c, i, k)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
range_ro = 1:2:R4;
range_co = 2:2:C4 - 2;
range_cr = 1:2:C4 - 2;
range_cl = 3:2:C4;

Y4(range_ro, range_co) = floor((Y4(range_ro, range_cr) + Y4(range_ro, range_cl) + 1)/2);

%   Column quarter pixels (d, n, f, q)
range_ro = 2:2:R4-2;
range_co = 1:2:C4;
range_ru = 1:2:R4-2;
range_rl = 3:2:R4;
Y4(range_ro, range_co) = floor((Y4(range_ru, range_co) + Y4(range_rl, range_co) + 1)/2);

%   Quarter pixels interpolated as /
%   Pixels starting from row 2 (e)
Y4(2:PAD_FACTOR:end, 2:PAD_FACTOR:end) = ...
    floor((Y4(1:PAD_FACTOR:end, 3:PAD_FACTOR:end) + Y4(3:PAD_FACTOR:end, 1:PAD_FACTOR:end) + 1)/2);

%   Pixels starting from row 4 (r)
Y4(4:PAD_FACTOR:end-1, 4:PAD_FACTOR:end-1) = ...
    floor((Y4(5:PAD_FACTOR:end, 3:PAD_FACTOR:end-2) + Y4(3:PAD_FACTOR:end-2, 5:PAD_FACTOR:end) + 1)/2);


%   Quarter pixels interpolated as \
%   Pixels starting from row 2 (g)
Y4(2:PAD_FACTOR:end-1, 4:PAD_FACTOR:end-1) = ...
    floor((Y4(1:PAD_FACTOR:end, 3:PAD_FACTOR:end-2) + Y4(3:PAD_FACTOR:end, 5:PAD_FACTOR:end) + 1)/2);

%   Pixels starting from row 4 (p)
Y4(4:PAD_FACTOR:end-1, 2:PAD_FACTOR:end-1) = ...
    floor((Y4(3:PAD_FACTOR:end-2, 1:PAD_FACTOR:end) + Y4(5:PAD_FACTOR:end, 3:PAD_FACTOR:end) + 1)/2);


%   Bottom row and rightmost column
i = find(Y4(end, 1:end-1) == 0);
b_row_minus1 = Y4(end-1, :);
Y4(end, i) = b_row_minus1(i);

i = find(Y4(:, end) == 0);
r_col_minus1 = Y4(:, end-1);
Y4(i, end) = r_col_minus1(i);
