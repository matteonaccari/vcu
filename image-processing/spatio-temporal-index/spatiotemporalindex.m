function [SI, TI] = spatiotemporalindex(sequence_name, R, C, chroma_format, input_bit_depth, frame_range)
%   Spatial Index (SI) and Temporal Index (TI) calculator, version 1.0
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
%   The spatiotemporalindex software returns the SI and TI information
%   associated to the video content whose YCbCr file name is passed as input
%   parameter. The spatial and temporal information is computed according
%   to the specification given in: ITU-T, "Subjective video quality
%   assessment methods for multimedia applications", Recommendation ITU-T
%   P 910, September 1999.
%   WARNING: When a YCbCr is presented as input, planar format is assumed
%
%   Parameters:
%       - Input:
%           sequence_name    = string containing the name of the YCbCr video
%                             sequence whereby a 4:2:0 chrominance
%                             subsampling factor is assumed
%           R                = number of rows for each video frame
%           C                = number of columns for each video frame
%           chroma_format    = chroma subsampling format (4:0:0, 4:2:0, 4:2:2 or 4:4:4)
%           input_bit_depth  = sample bit depth
%           frame_range      = range/number of frames to be processed. Range of frames are
%                              expressed as start_idx:stop_idx where the index starts from zero
%
%       - Output:
%           SI               = variable containing the SI associated to the
%                              video content
%           SI               = variable containing the TI associated to the
%                              video content

%   Open the video content
fp = fopen(sequence_name, 'r');

if fp == -1
    error(['Sequence ' sequence_name ' does not exist']);
end

switch input_bit_depth
case 8
    bytesPerSample = 1;
    precisionString = 'uint8';
case {9,10,11,12,13,14,15,16}
    bytesPerSample = 2;
    precisionString = 'uint16';
otherwise
    error('Bits per sample %d not valid, abort...\n', input_bit_depth);
    fclose(fp);
end

%   Compute the offsets for frame counting and chroma skipping
switch chroma_format
case '4:0:0'
    bytes4Chroma = 0;
case '4:2:0'
    bytes4Chroma = bytesPerSample*R*C*0.5;
case '4:2:2'
    bytes4Chroma = bytesPerSample*R*C;
case '4:4:4'
    bytes4Chroma = bytesPerSample*R*C*2;
otherwise
    error('Unknown format %s, abort...\n', chroma_format);
    fclose(fp);
end

if length(frame_range) > 1
    % Input given as a range/vector of values
    F = frame_range;
else
    % Input given as scalar
    F = 0:frame_range-1;
end

%   Compute the Sobel window needed for SI computation
h = [1 2 1; 0 0 0; -1 -2 -1];
h = h/8;

%   Output vectors declaration
SI_array = zeros(length(F), 1);
TI_array = zeros(length(F), 1);

isRgb = strfind(sequence_name, '.rgb') > 1;

%   SI and TI processing loop
i = 1
for f = F
    %   Read current frame
    if isRgb
        fseek(fp, f*3*R*C*bytesPerSample, 'bof');
        Red = fread(fp, [C R], precisionString)';
        Green = fread(fp, [C R], precisionString)';
        Blue = fread(fp, [C R], precisionString)';
        Y = bt709Conversion(Red, Green, Blue, input_bit_depth);
    else
        index = f * (R*C*bytesPerSample + bytes4Chroma);
        fseek(fp, index, 'bof');
        Y = fread(fp, [C R], precisionString)';
    end

    %   Spatial information
    hor_edge = filter2(h, Y, 'valid');
    vert_edge = filter2(h', Y, 'valid');
    SI_array(i) = std(sqrt(hor_edge(:).^2 + vert_edge(:).^2));

    %   Temporal information
    if i > 1
        D = Y - Y_prev;
        TI_array(i) = std(D(:));
    end

    Y_prev = Y;
    i = i + 1;

end

%   SI-TI are divided to make them referring to 8-bit precision
SI = max(SI_array);
TI = max(TI_array);

fclose(fp);

function Y = bt709Conversion(R, G, B, input_bit_depth)
t = 0.2126*R + 0.7152*G + 0.0722*B;
Y   = floor (16 + 219 * t / 255 + 0.5);
maxValue = bitshift(1, input_bit_depth);
Y(Y > maxValue) = maxValue;
Y(Y < 0) = 0;