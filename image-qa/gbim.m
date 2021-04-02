function MGBIM = gbim(I, B)
%   No-reference Generalized Block Impairment Metric (GBIM), version 1.0
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
%   The gbim software computes the perceptual blockiness metric
%   proposed in H. R. Wu and M. Yuen, "A generalized block-edge impairment
%   metric for video coding", IEEE Signal Process. Lett., vol. 4, no. 11,
%   pp. 317-320, Nov. 1997.
%   The metric works in no-reference modality and it may be parametrized
%   with respect to the square image block dimension.
%   Each image is assumed as a 8-bit grey levels content.
%
%   Input parameters:
%       - I                = Luminance component for the image whose the GBIM
%                            is being computed
%       - B                = Block size to be considered in the GBIM
%                            computation
%
%   Output parameters:
%       - MGBIM            = Image level metric value

%   Declarations and assignments
[R C] = size(I);
xi = 81;
lambda = log(1 + sqrt(255 - xi))/log(1 + sqrt(xi));

%   Dimensions check
if rem(R, B) ~= 0
    error('Number of rows (%d) is not a multiple of block size (%d)', R, B);
end
if rem(C, B) ~= 0
    error('Number of columns (%d) is not a multiple of block size (%d)', R, B);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Horizontal blockiness
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Compute the Dcf matrix as in equation (1)
right_c = B:B:C-B;
left_c  = B+1:B:C-B+1;

image_col_right = I(:, right_c);
image_col_left  = I(:, left_c);

Dcf = image_col_right - image_col_left;

%   Compute the mu(i,j) and the sigma(i,j) terms
mu    = zeros(R, C/B-1);
sigma = zeros(R, C/B-1);
for c = 1:C/B - 1
    left_col  = I(:, (c-1)*B + 1:c*B);
    right_col = I(:, c*B + 1:(c+1)*B);
    mu_left   = mean(left_col, 2);
    mu_right  = mean(right_col, 2);
    mu(:, c) = (mu_left + mu_right)/2;

    sigma_left  = std(left_col, 1, 2);
    sigma_right = std(right_col, 1, 2);

    sigma(:, c) = (sigma_left + sigma_right)/2;
end

%   Compute the intermediate terms for the weight w
index_low = find(mu <= xi);
index_up = find(mu > xi);

%   arg1 = 1 + sqrt(mu)/(1 + sigma). For mu(i, j) <= xi
arg1 = 1 + sqrt(mu)./(1 + sigma);

%   arg2 = 1 + sqrt(255 - mu)/(1 + sigma). For mu(i, j) > xi
arg2 = 1 + sqrt(255 - mu)./(1 + sigma);

%   low_cond = lambda*log(arg1);
low_cond = lambda * log(arg1);

%   up_cond = log(arg2)
up_cond  = log(arg2);

%   Compute the term w as in equation (3)
w = zeros(R, C/B-1);
w(index_low) = low_cond(index_low);
w(index_up)  = up_cond(index_up);

%   Compute the Mh matrix as in equation (1)
temp = w.*Dcf;
Mh = sqrt(sum(sum(temp.*temp)));

%   Compute the matrix E as in equation (5)
S = zeros(1, B-1);
for n = 1:B-1
    current = I(:, B + n: B :end);
    next    = I(:, B + n + 1: B :end);
    D       = current - next;
    temp    = w.*D;
    S(n)    = sqrt(sum(sum(temp.*temp)));
end

E = mean(S);

%   Compute MhGBIM
MhGBIM = Mh / E;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Vertical blockiness
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   Compute the vertical Dcf as in equation (1)
top_r = B:B:R-B;
bot_r = B+1:B:R-B+1;

image_row_top = I(top_r, :);
image_row_bot = I(bot_r, :);

Dcf = image_row_top - image_row_bot;

%   Compute the mu(i,j) and the sigma(i,j) terms
mu    = zeros(R/B-1, C);
sigma = zeros(R/B-1, C);
for r = 1:R/B - 1
    top_row = I((r-1)*B + 1:r*B, :);
    bot_row = I(r*B + 1:(r+1)*B, :);
    mu_top  = mean(top_row, 1);
    mu_bot  = mean(bot_row, 1);
    mu(r, :) = (mu_top + mu_bot)/2;

    sigma_top = std(top_row, 1, 1);
    sigma_bot = std(bot_row, 1, 1);

    sigma(r, :) = (sigma_top + sigma_bot)/2;
end

%   Compute the intermediate terms for the weight w
index_low = find(mu <= xi);
index_up  = find(mu > xi);

%   arg1 = 1 + sqrt(mu)/(1 + sigma). For mu(i, j) <= xi
arg1 = 1 + sqrt(mu)./(1 + sigma);

%   arg2 = 1 + sqrt(255 - mu)/(1 + sigma). for mu(i, j) > xi
arg2 = 1 + sqrt(255 - mu)./(1 + sigma);

%   low_cond = lambda*log(arg1);
low_cond = lambda * log(arg1);

%   up_cond = log(arg2)
up_cond  = log(arg2);

%   Compute the weight w as in equation (3)
w = zeros(R/B - 1, C);
w(index_low) = low_cond(index_low);
w(index_up)  = up_cond(index_up);

%   Compute the Mv matrix as in equation (2)
temp = w.*Dcf;
Mv = sqrt(sum(sum(temp.*temp)));

%   Compute the matrix E as in equation (5)
S = zeros(B-1, 1);
for n = 1:B-1
    current = I(B + n: B :end, :);
    next    = I(B + n + 1: B :end, :);
    D       = current - next;
    temp    = w.*D;
    S(n)    = sqrt(sum(sum(temp.*temp)));
end

E = mean(S);

%   Compute the MvGBIM
MvGBIM = Mv / E;

%   Compute the final MGBIM at the frame level
MGBIM = 0.5*MhGBIM + 0.5*MvGBIM;
