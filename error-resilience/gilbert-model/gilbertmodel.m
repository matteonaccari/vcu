function pattern = gilbertmodel(plr, bl)

%   gilbertmodel, version 1.0
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
%   The gilbertmodel software generates an error pattern whereby the
%   errors are introducted according to a two state Gilbert model:
%
%                 _______       Pgb           _______
%          ------|       |------------------>|       |------
%      Pgg |     |   G   |                   |   B   |     | Pbb
%          ----->|_______|<------------------|_______|<-----
%                               Pbg
%
%   where:
%   Pgg = probability of good remaining good
%   Pbb = probability of bad remaining bad
%   Pgb = probability of good becoming bad
%   Pbg = probability of bad becoming good
%   
%   Therefore, the probability of loosing a coded packet is:
%                 --------------------------
%                 | PLR = Pgb/(Pbg + Pgb)  |
%                 --------------------------
%
%   Parameters:
%       - Input:
%           plr     = packet loss rate in percentage [0 - 100]
%           bl      = burst length (in packets)
%       - Output:
%           pattern = column vector associated to the error pattern. A zero
%                     value in the vector means no error occurred.
%                     Conversely, a one value means error occurred.
%   Note:
%       The error pattern generated assumes the following convention: 0 values
%       are associated with correct transmission whilst 1 values denote packet drops.
%       A burst of channel errors is defined as a contiguous sequence of 2
%       or more '1' values.

b = 1 - 1/bl;
p = plr/100;
g = 1 - ((1-b)/(1-p))*p;

N = 10000;

j = 1;


rand('state');

pattern = NaN*ones(N, 1);
pattern(1) = 0; %   Assumes to start in the good state

for i=2:N,
    if pattern(i-1) == 0
        if rand(1) <= g
            pattern(i) = 0;
        else
            pattern(i) = 1;
        end
    else
        if rand(1) <= b
            pattern(i) = 1;
        else
            pattern(i) = 0;
        end
    end
end

f = find(pattern == 1);

% Find the burst length
i = 1;

burst = [];

while i <= N

    if(pattern(i) == 1)
        j = i + 1;
        l = 1;
        while j <= N && pattern(j) == 1
            l = l + 1;
            j = j + 1;
        end
        if l > 1
            burst = [burst; l];
        end
        i = j - 1;
    end

    i = i + 1;

end

L = mean(burst);
aplr = length(pattern(f))/N*100;

display(['True mean PLR [%]: ' num2str(aplr)])
display(['True burst length [packets]: ' num2str(L)])
