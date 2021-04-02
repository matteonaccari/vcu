function burst = burstcounter(pattern_file)
%   burstcounter, version 1.0
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
%   The burstcounter software returns the information associated with the burst of 
%   transmission errors contained in the pattern file specified as input
%
%   Parameters:
%       - Input:
%           pattern_file = string containing the name of the error pattern
%                          file
%       - Output:
%           burst        = matrix of # x 3 elements where # denotes the
%                          number of bursts found in the error pattern file.
%                          The columns of the burst matrix have the
%                          following meaning: 1) the burst length of the
%                          i-th burst, 2)-3) the starting and the end
%                          points of the i-th burst
%   Note:
%       The error patter file must contain only '0' and '1' ASCII
%       characters (8 bits). The character '0' means that no channel error
%       occurred whilst the character '1' means that a channel error
%       occurred. A burst of channel errors is defined as a contiguous sequence of 2
%       or more characters '1'.


fp = fopen(pattern_file, 'rb');

if fp == -1
	error(sprintf('File %s does not exist, abort!\n', pattern_file));
end

pattern = fread(fp);

fclose(fp);

pattern(pattern == 48) = 0;
pattern(pattern == 49) = 1;

burst = [];

i = 1;

tot_length = length(pattern);

while i <= tot_length

    if pattern(i) == 1
        j = i + 1;
        l = 1;
        start = i - 1;
        while j <= tot_length && pattern(j) == 1
            l = l + 1;
            j = j + 1;
        end
        if l > 1
            burst = [burst; [l start start + l - 1]];
        end
        i = j - 1;
    end

    i = i + 1;

end

fprintf('Average burst length: %.2f\n', mean(burst(:, 1)));