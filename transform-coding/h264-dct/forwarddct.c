/*  Mex functions to compute the forward H.264/AVC 4x4 integer DCT and quantisation, version 1.0
 *  Copyright(c) 2021 Matteo Naccari (matteo.naccari@gmail.com)
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

#include "definitions.h"
/*!
 *
 *  \brief
 *  Mex function wrapper: this is the interface between Matlab and the DCT computation function
 *
 *  \param[in]  nlhs: number of returned parameters, i.e. parameters and the left hand side of the Matlab
 *                    call for this MEX function
 *  \param[in]  plhs: pointer to the returned parameters
 *  \param[in]  nrhs: number of input parameters, i.e. parameters and the right hand side of the Matlab
 *                    call for this MEX function
 *  \param[in]  prhs: pointer to the input parameters
 *
 *  \author
 *  Matteo Naccari (matteo.naccari@gmail.com)
 *
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  //All data are passed as double
  double *source_image, *output_image_dct;

  source_image = mxGetPr(prhs[0]);

  rows = (unsigned int)mxGetM(prhs[0]);
  columns = (unsigned int)mxGetN(prhs[0]);

  QP = (unsigned int)mxGetScalar(prhs[1]);

  //  DCT matrix allocation
    plhs[0] = mxCreateDoubleMatrix(rows, columns, mxREAL);
    output_image_dct = mxGetPr(plhs[0]);

  forward_dct(source_image, output_image_dct);
}

/*!
 *
 *  \brief
 *  Extracts a 4x4 block from the image being transformed and quantized
 *
 *  \param[in]  source_image: pointer to the input image which is organized as a 1D array.
 *  \param[in]  r: row coordinate at which start to extract the image pixels. It is provided in 4x4 block
 *                 granularity units
 *  \param[in]  c: column coordinate at which start to extract the image pixels. It is provided in 4x4 block
 *                 granularity units
 *  \param[out]  output_block: 4x4 output block cointaining the image pixel values
 *
 *  \author
 *  Matteo Naccari (matteo.naccari@gmail.com)
 *
*/
void extract_4x4_block(double *source_image, int output_block[4][4], unsigned int r, unsigned int c)
{
  unsigned int i, j, rr, cc;

  rr = r << 2;
  cc = c << 2;

  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      output_block[i][j] = (int)source_image[(cc + j)*rows + rr + i];
    }
  }
}

/*!
 *
 *  \brief
 *  Computes the forward core transform (only add and shifts) T*block*T'
 *
 *  \param[in]  block: input 4x4 block
 *  \param[out]  transformed: 4x4 output block
 *
 *  \author
 *  Matteo Naccari (matteo.naccari@gmail.com)
 *
*/
void forward_core_trasform(int block[4][4], int transformed[4][4])
{
  unsigned int r, c, j;
  int sum, temp[4][4];

  //Main operation: T*block*T'

  //Step 1: temp = T*block
  for(r = 0; r < 4; r++) {
    for(c = 0; c < 4; c++) {
      sum = 0;
      for(j = 0; j < 4; j++)
        sum += T_forward[r][j]*(block[j][c]);
      temp[r][c] = sum;
    }
  }

  //Step 2: transformed = temp*T'
  for(r = 0; r < 4; r++) {
    for(c = 0; c < 4; c++) {
      sum = 0;
      for(j = 0; j < 4; j++)
        sum += temp[r][j]*T_forward[c][j];
      transformed[r][c] = sum;
    }
  }
}

/*!
 *
 *  \brief
 *  Multiplies the core transformed coefficients to make the transform orthonormal
 *
 *  \param[in]  block_transformed: input 4x4 block of the transformed coefficients
 *  \param[out]  block_scaled: output 4x4 block of the scaled coefficients
 *
 *  \author
 *  Matteo Naccari (matteo.naccari@gmail.com)
 *
*/
void post_scaling(int block_transformed[4][4], int block_scaled[4][4])
{
  unsigned int QP_rem = QP%6;
  unsigned int QP_quo = QP/6;
  unsigned int qbits  = 15 + QP_quo;
  unsigned int r, c;

  for(r = 0; r < 4; r++)
    for(c = 0; c < 4; c++) {
      if(block_transformed[r][c] >= 0)
        block_scaled[r][c] = (int)(abs(block_transformed[r][c])*
                   POSTSCALING_FACTOR[QP_rem][r][c] + f[QP_quo]) >> qbits;
      else
        block_scaled[r][c] = -(int)((abs(block_transformed[r][c])*
                   POSTSCALING_FACTOR[QP_rem][r][c] + f[QP_quo]) >> qbits);
    }
}

/*!
 *
 *  \brief
 *  Puts a transformed 4x4 block in the trasformed image
 *
 *  \param[in]  source: input 4x4 block of the transformed coefficients
 *  \param[in]  r: row coordinate at which start to put the transformed coefficients. It is provided in 4x4
 *                 block granularity units
 *  \param[in]  r: column coordinate at which start to put the transformed coefficients. It is provided in 4x4
 *                 block granularity units
 *  \param[out]  destination: pointer to the transformed image
 *
 *  \author
 *  Matteo Naccari (matteo.naccari@gmail.com)
 *
*/
void copy_4x4_block(double *destination, int source[4][4], unsigned int r, unsigned int c)
{
  unsigned int i, j, rr, cc;

  rr = r << 2;
  cc = c << 2;

  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      destination[(cc + j)*rows + rr + i] = source[i][j];
    }
  }
}

/*!
 *
 *  \brief
 *  Performs the H.264/AVC 4x4 DCT and quantisation over the whole image
 *
 *  \param[in]  source_image: input image
 *  \param[out]  output_image_dct: output transformed image
 *
 *  \author
 *  Matteo Naccari (matteo.naccari@gmail.com)
 *
*/
void forward_dct(double *source_image, double *output_image_dct)
{
  unsigned int block_rows, block_columns;
  unsigned int r, c, i, j;
  int block[4][4];

  int block_core_transformed[4][4], block_transformed[4][4];


  block_rows = rows >> 2;
  block_columns = columns >> 2;


  //Forward integer DCT
  for(r = 0; r < block_rows; r++) {
    for(c = 0; c < block_columns; c++) {

      //4x4 block reading
      extract_4x4_block(source_image, block, r, c);

      //Foward core transform
      forward_core_trasform(block, block_core_transformed);

      //Post-scaling and shift operations
      post_scaling(block_core_transformed, block_transformed);

      //Copy the transformed block
      copy_4x4_block(output_image_dct, block_transformed, r, c);
    }
  }
}