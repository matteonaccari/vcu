/*  Mex functions to compute the inverse H.264/AVC 4x4 integer DCT and quantisation, version 1.0
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
 *  Mex function wrapper: this is the interface between Matlab and the inverse DCT computation function
 *
 *  \param[in]  nlhs: number of returned parameters, i.e. parameters and the left hand side of the Matlab
 *                    call for this MEX function
 *  \param[in]  plhs: pointer to the returned parameters
 *  \param[in]  nrhs: number of input parameters, i.e. parameters and the right hand side of the Matlab
 *                    call for this MEX function
 *  \param[in]  prhs: pointer to the input parameters
 *
 *  \author
 *  Matteo Naccari
 *
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  //All data are passed as double
  double *input_dct_image, *reconstructed_image;

  input_dct_image = mxGetPr(prhs[0]);

  rows = (unsigned int)mxGetM(prhs[0]);
  columns = (unsigned int)mxGetN(prhs[0]);

  QP = (unsigned int)mxGetScalar(prhs[1]);

  //  Reconstructed image matrix allocation
    plhs[0] = mxCreateDoubleMatrix(rows, columns, mxREAL);
    reconstructed_image = mxGetPr(plhs[0]);

  inverse_dct(input_dct_image, reconstructed_image);
}

/*!
 *
 *  \brief
 *  Extracts a 4x4 block from the image being transformed and quantized
 *
 *  \param[in]  transformed_image: pointer to the transformed image orgainzed as a 1D array
 *  \param[in]  r: row coordinate at which start to extract the transformed coefficients. It is
 *                 provided in 4x4 block granularity units
 *  \param[in]  c: column coordinate at which start to extract the transformed coefficients. It is
 *                 provided in 4x4 block granularity units
 *  \param[out]  block_dct: 4x4 output block cointaining the transformed coefficients
 *
 *  \author
 *  Matteo Naccari
 *
*/
void extract_4x4_block(double *transformed_image, int block_dct[4][4], unsigned int r, unsigned int c)
{
  unsigned int i, j, rr, cc;

  rr = r << 2;
  cc = c << 2;

  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      block_dct[i][j] = (int)transformed_image[(cc + j)*rows + rr + i];
    }
  }
}

/*!
 *
 *  \brief
 *  Performs the coefficients rescaling for the inverse   quantisation
 *
 *  \param[in]  input_block: input 4x4 block of the transformed coefficients
 *  \param[out]  output_block: output 4x4 block of the rescaled coefficients
 *
 *  \author
 *  Matteo Naccari
 *
*/
void rescaling(int input_block[4][4], int output_block[4][4])
{
  unsigned int r, c;
  unsigned int QP_rem = QP%6;
  unsigned int QP_quo = QP/6;

  for(r = 0; r < 4; r++)
    for(c = 0; c < 4; c++)
      output_block[r][c] = (input_block[r][c]*RESCALING_FACTOR[QP_rem][r][c]) << QP_quo;

}

/*!
 *
 *  \brief
 *  Computes the forward core transform (only add and shifts)
 *
 *  \param[in]  input_block: input 4x4 block of the rescaled coefficients
 *  \param[out]  output_block: 4x4 output block of the reconstructed pixels
 *
 *  \author
 *  Matteo Naccari
 *
*/
void inverse_core_transform(int input_block[4][4], int output_block[4][4])
{
    int i, s02, d02, s13, d13;

    // horizontal
    for (i = 0; i < 4; i++)
    {
        s02 = (input_block[i][0] + input_block[i][2]);
        d02 = (input_block[i][0] - input_block[i][2]);
        s13 = input_block[i][1] + (input_block[i][3] >> 1);
        d13 = (input_block[i][1] >> 1) - input_block[i][3];

        output_block[i][0] = (s02 + s13);
        output_block[i][1] = (d02 + d13);
        output_block[i][2] = (d02 - d13);
        output_block[i][3] = (s02 - s13);
    }
    // vertical
    for (i = 0; i < 4; i++)
    {
        s02 = (output_block[0][i] + output_block[2][i]);
        d02 = (output_block[0][i] - output_block[2][i]);
        s13 = output_block[1][i] + (output_block[3][i] >> 1);
        d13 = (output_block[1][i] >> 1) - output_block[3][i];

        output_block[0][i] = ((s02 + s13 + 32) >> 6);
        output_block[1][i] = ((d02 + d13 + 32) >> 6);
        output_block[2][i] = ((d02 - d13 + 32) >> 6);
        output_block[3][i] = ((s02 - s13 + 32) >> 6);
    }
}

/*!
 *
 *  \brief
 *  Puts a transformed 4x4 block in the trasformed image
 *
 *  \param[in]  input_block: input 4x4 block of the reconstructed pixels
 *  \param[in]  r: row coordinate at which start to put the reconstructed pixels. It is provided in 4x4
 *                 block granularity units
 *  \param[in]  r: column coordinate at which start to put the reconstructed pixels. It is provided in 4x4
 *                 block granularity units
 *  \param[out]  destination: pointer to the reconstructed pixels
 *
 *  \author
 *  Matteo Naccari
 *
*/
void copy_4x4_block(double *output_image, int input_block[4][4], unsigned int r, unsigned int c)
{
  unsigned int i, j, rr, cc;

  rr = r << 2;
  cc = c << 2;

  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      output_image[(cc + j)*rows + rr + i] = input_block[i][j];
    }
  }
}

/*!
 *
 *  \brief
 *  Performs the H.264/AVC 4x4 inverse DCT and inverse   quantisation over the whole transformed image
 *
 *  \param[in]  source_image: input image
 *  \param[out]  output_image_dct: output transformed image
 *
 *  \author
 *  Matteo Naccari
 *
*/
void inverse_dct(double *input_dct_image, double *reconstructed_image)
{
  unsigned int block_rows, block_columns;
  unsigned int r, c, i, j;
  int reconstructed_block[4][4];
  int block_dct[4][4], block_dct_rescaled[4][4];

  int block_core_transformed[4][4], block_transformed[4][4];

  block_rows = rows >> 2;
  block_columns = columns >> 2;

  //Inverse integer DCT
  for(r = 0; r < block_rows; r++) {
    for(c = 0; c < block_columns; c++) {

      //4x4 block reading
      extract_4x4_block(input_dct_image, block_dct, r, c);

      //Rescaling
      rescaling(block_dct, block_dct_rescaled);

      //Inverse core transformed
      inverse_core_transform(block_dct_rescaled, reconstructed_block);

      //Copy the reconstructed block
      copy_4x4_block(reconstructed_image, reconstructed_block, r, c);
    }
  }
}