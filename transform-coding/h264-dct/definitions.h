#ifndef DEFINITION_HH
#define DEFINITION_HH

/*  Mex functions to compute the forward and inverse H.264/AVC 4x4 integer DCT and quantisation, version 1.0
 *  Copyright(c) 2021 Matteo Naccari
 *  All Rights Reserved.
 *
 *  email: matteo.naccari@gmail.com | matteo.naccari@polimi.it | matteo.naccari@gmail.com
 *
 *  The copyright in this software is being made available under the BSD
 *  License, included below. This software may be subject to other third party
 *  and contributor rights, including patent rights, and no such rights are
 *  granted under this license.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of the author may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "mex.h"

/*!
 *
 *  \brief
 *  MEX function header file
 *
 *
 *  \author
 *  Matteo Naccari {matteo.naccari@gmail.com}
 *
*/

//!  Clipping of the variable c in the range [a,b]
#define CLIP(a, b, c) a < b ? b : a > c ? c : a

//!  Multipliers to perform inverse quantisation
const int RESCALING_FACTOR[6][4][4] =
{
  {{10,13,10,13}, {13,16,13,16}, {10,13,10,13}, {13,16,13,16}},
  {{11,14,11,14}, {14,18,14,18}, {11,14,11,14}, {14,18,14,18}},
  {{13,16,13,16}, {16,20,16,20}, {13,16,13,16}, {16,20,16,20}},
  {{14,18,14,18}, {18,23,18,23}, {14,18,14,18}, {18,23,18,23}},
  {{16,20,16,20}, {20,25,20,25}, {16,20,16,20}, {20,25,20,25}},
  {{18,23,18,23}, {23,29,23,29}, {18,23,18,23}, {23,29,23,29}}
};

//!  Number of rows which constitutes the input matrix
//!  Warning: it is assumed that rows is a multiple of 4
unsigned int rows;

//!  Number of columns which constitutes the input matrix
//!  Warning: it is assumed that columns is a multiple of 4
unsigned int columns;

//!  quantisation parameter used over the whole frame
unsigned int QP;

//!  Selects a 4x4 block from the image being inverse transformed and inverse quantized
void extract_4x4_block(double*, int[4][4], unsigned int, unsigned int);

//!  Performs the coefficients rescaling for the inverse quantisation
void rescaling(int [4][4], int[4][4]);

//!  Performs the inverse core transform (only additions and shifts)
void inverse_core_transform(int [4][4], int[4][4]);

//!  Puts a 4x4 block of pixels in the output
void copy_4x4_block(double*, int[4][4], unsigned int, unsigned int);

//!  Performs the whole inverse DCT and inverse quantisation process
void inverse_dct(double *, double *);

//!  Offset used during rounding operations
const int f[13] = {10912, 21824, 43648, 87296, 174592, 349184, 698368, 1396736, 2793472, 5586944, 11173888, 22347776};

//!  Multiplication factors used to make the transform orthonormal
const int POSTSCALING_FACTOR[6][4][4] = {
  {{13107, 8066, 13107, 8066},{8066, 5243, 8066, 5243},{13107, 8066, 13107, 8066},{8066, 5243, 8066, 5243}},
  {{11916, 7490, 11916, 7490},{7490, 4660, 7490, 4660},{11916, 7490, 11916, 7490},{7490, 4660, 7490, 4660}},
  {{10082, 6554, 10082, 6554},{6554, 4194, 6554, 4194},{10082, 6554, 10082, 6554},{6554, 4194, 6554, 4194}},
  {{9362, 5825, 9362, 5825},{5825, 3647, 5825, 3647},{9362, 5825, 9362, 5825},{5825, 3647, 5825, 3647}},
  {{8192, 5243, 8192, 5243},{5243, 3355, 5243, 3355},{8192, 5243, 8192, 5243},{5243, 3355, 5243, 3355}},
  {{7282, 4559, 7282, 4559},{4559, 2893, 4559, 2893},{7282, 4559, 7282, 4559},{4559, 2893, 4559, 2893}}
};

//!  Core forward transform kernel matrix
const int T_forward[4][4] =
{
    {1, 1, 1, 1},
    {2, 1, -1, -2},
    {1, -1, -1, 1},
    {1, -2, 2, -1}
};

//!  Number of rows which constitutes the input matrix
//!  Warning: it is assumed that rows is a multiple of 4
unsigned int rows;

//!  Number of columns which constitutes the input matrix
//!  Warning: it is assumed that columns is a multiple of 4
unsigned int columns;

//!  Selects a 4x4 block from the image being transformed and quantized
void extract_4x4_block(double*, int[4][4], unsigned int, unsigned int);

//!  Performs the forward core transform (only add and shifts) T*block*T'
void forward_core_trasform(int[4][4], int[4][4]);

//!  Multiplies the core transformed coefficients to make the transform orthonormal
void post_scaling(int[4][4], int[4][4]);

//!  Puts a transformed 4x4 block in the trasformed image
void copy_4x4_block(double*, int[4][4], unsigned int, unsigned int);

//!  Performs the whole forward DCT and quantisation process
void forward_dct(double*, double*);
#endif