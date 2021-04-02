/*  spatiotemporalindex, version 1.0
 *  Copyright(c) 2021 Matteo Naccari
 *  All Rights Reserved.
 *
 *  email: matteo.naccari@gmail.com | matteo.naccari@polimi.it | matteo.naccari@lx.it.pt
 *
 * The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the author may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The spatiotemporalindex software returns the SI and TI information
 * associated to the video content whose YCbCr file name is passed as input
 * parameter. The spatial and temporal information is computed according
 * to the specification given in: ITU-T, "Subjective video quality
 * assessment methods for multimedia applications", Recommendation ITU-T
 * P 910, September 1999.
 * WARNING: When a YCbCr is presented as input, planar format is assumed
*/

#include <iostream>
#include <string>
#include <iomanip>
#include <exception>
#include "spatialtemporalindex.h"

using namespace std;

int main(int argc, char **argv)
{
  int retCode = EXIT_SUCCESS;
  if (argc < 6) {
    cout << "Usage " << argv[0] << "<input_file> <frame_height> <frame_width> <chroma_format> <bit_depth> <frame_range>" << endl;
    cout << "\t <input_file>   : Input in planar format YUV or RGB. For RGB, BT.709 color space is assumed" << endl;
    cout << "\t <frame_height> : Frame height in luma samples" << endl;
    cout << "\t <frame_width>  : Frame width in luma samples" << endl;
    cout << "\t <chroma_format>: Chroma format specified as integer, e.g. 420 for 4:2:0" << endl;
    cout << "\t <bit_depth>    : Bit depth of the input file" << endl;
    cout << "\t <frame_range>  : Number of frames to be processed specified as integer value or range of integers start:stop" << endl;
    return retCode;
  }

  const string inputSequence(argv[1]);
  const string framesToBeProcessed(argv[6]);
  const int frameHeight  = atoi(argv[2]);
  const int frameWidth   = atoi(argv[3]);
  const int chromaFormat = atoi(argv[4]);
  const int bitDepth     = atoi(argv[5]);

  // Determine the frame range
  const size_t offset = inputSequence.length() - 4;
  bool isRgb = inputSequence.find(".rgb", offset) != string::npos;
  size_t colonPosition = framesToBeProcessed.find(':', 0);
  int startIdx, stopIdx;
  bool invalidFrameRange = false;

  try {
    if (colonPosition != string::npos) {
      size_t rangeStringLength = framesToBeProcessed.length();
      startIdx = atoi(framesToBeProcessed.substr(0, colonPosition).c_str());
      stopIdx = atoi(framesToBeProcessed.substr(colonPosition + 1, rangeStringLength - colonPosition - 1).c_str());
      invalidFrameRange = startIdx < 0 || stopIdx < 0 || startIdx > stopIdx;
    }
    else {
      startIdx = 0;
      stopIdx = atoi(argv[6]);
      invalidFrameRange = stopIdx < 0;
    }

    if (invalidFrameRange) {
      throw runtime_error("Invalid frame range: " + framesToBeProcessed);
    }

    cout << "------------------------------------------" << endl;
    cout << "| Frame | Spatial index | Temporal index |" << endl;
    cout << "------------------------------------------" << endl;

    if (bitDepth == 8) {
      SpatialTemporalIndex<uint8_t> siTiIndexEngine;
      siTiIndexEngine.init(frameHeight, frameWidth, chromaFormat, startIdx, bitDepth, inputSequence, isRgb);

      for (int idx = startIdx; idx < stopIdx; idx++) {
        siTiIndexEngine.fetchNewFrame();

        siTiIndexEngine.computeSpatialIndex(idx);

        if (idx > startIdx) {
          siTiIndexEngine.computeTemporalIndex(idx);
        }

        siTiIndexEngine.swapFrames();

        cout << "| " << setw(5) << idx << " | " << setw(13) << setprecision(8) << siTiIndexEngine.getCurrentSpatialIdx()
          << " | " << setw(14) << setprecision(8) << siTiIndexEngine.getCurrentTemporalIdx() << " |" << endl;
        cout.flush();
      }
      cout << endl << endl << "Sequence level SI/TI: " << siTiIndexEngine.getSpatialIdx() << " / " << siTiIndexEngine.getTemporalIdx() << endl;
    }
    else {
      SpatialTemporalIndex<uint16_t> siTiIndexEngine;
      siTiIndexEngine.init(frameHeight, frameWidth, chromaFormat, startIdx, bitDepth, inputSequence, isRgb);

      for (int idx = startIdx; idx < stopIdx; idx++) {
        siTiIndexEngine.fetchNewFrame();

        siTiIndexEngine.computeSpatialIndex(idx);

        if (idx > startIdx) {
          siTiIndexEngine.computeTemporalIndex(idx);
        }

        siTiIndexEngine.swapFrames();

        cout << "| " << setw(5) << idx << " | " << setw(13) << setprecision(8) << siTiIndexEngine.getCurrentSpatialIdx()
          << " | " << setw(14) << setprecision(8) << siTiIndexEngine.getCurrentTemporalIdx() << " |" << endl;
        cout.flush();
      }
      cout << endl << endl << "Sequence level SI/TI: " << siTiIndexEngine.getSpatialIdx() << " / " << siTiIndexEngine.getTemporalIdx() << endl;
    }
  }
  catch (exception e) {
    cerr << "Something went wrong: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return retCode;
}