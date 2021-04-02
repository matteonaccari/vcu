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

#ifndef __SPATIAL_TEMPORAL_INDEX__
#define __SPATIAL_TEMPORAL_INDEX__

#include <iostream>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>
#include <fstream>
#include <exception>

using namespace std;

template <class BD>
class SpatialTemporalIndex
{
private:
  bool       m_isRgb;
  int        m_frameHeight;
  int        m_frameWidth;
  int        m_chromaFormat;
  int        m_bytesPerFrame;
  int        m_bytesChroma;
  int        m_maxPixelValue;
  int        m_bitDepth;
  BD        *m_frameDataCurrent;
  BD        *m_frameDataPrevious;
  FILE      *m_inputFileHandle;
  double     m_currentSpatialIdx;
  double     m_maxSpatialIdx;
  double     m_currentTemporalIdx;
  double     m_maxTemporalIdx;
  int        Clip3(const int minValue, const int maxValue, const int value)
  {
    return std::max<int>(minValue, std::min<int>(maxValue, value));
  }
public:
  SpatialTemporalIndex() :
    m_isRgb(false),
    m_frameHeight(0),
    m_frameWidth(0),
    m_chromaFormat(-1),
    m_bytesPerFrame(0),
    m_bytesChroma(0),
    m_maxPixelValue(0),
    m_bitDepth(0),
    m_frameDataCurrent(nullptr),
    m_frameDataPrevious(nullptr),
    m_inputFileHandle(nullptr),
    m_currentSpatialIdx(0.0),
    m_maxSpatialIdx(numeric_limits<double>::min()),
    m_currentTemporalIdx(0.0),
    m_maxTemporalIdx(numeric_limits<double>::min()){}

  ~SpatialTemporalIndex()
  {
    if (m_frameDataCurrent) {
      delete[] m_frameDataCurrent;
      m_frameDataCurrent = nullptr;
    }
    if (m_frameDataPrevious) {
      delete[] m_frameDataPrevious;
      m_frameDataPrevious = nullptr;
    }
    if (m_inputFileHandle) {
      fclose(m_inputFileHandle);
    }
  }

  double getCurrentSpatialIdx()  { return m_currentSpatialIdx;  }
  double getCurrentTemporalIdx() { return m_currentTemporalIdx; }
  double getSpatialIdx()         { return m_maxSpatialIdx;      }
  double getTemporalIdx()        { return m_maxTemporalIdx;     }

  void swapFrames()
  {
    BD *temp = m_frameDataCurrent;
    if (m_frameDataCurrent == nullptr || m_frameDataPrevious == nullptr) {
      throw runtime_error("Error trying to swap a null pointer");
    }
    m_frameDataCurrent  = m_frameDataPrevious;
    m_frameDataPrevious = temp;
  }

  void fetchNewFrame()
  {
    if (m_inputFileHandle == nullptr) {
      throw runtime_error("Error the input file is not opened");
    }

    const int elementsToRead = m_frameHeight * m_frameWidth;

    if (m_isRgb) {
      int errorCode = 0;
      BD *r = new BD[m_frameHeight * m_frameWidth];
      BD *g = new BD[m_frameHeight * m_frameWidth];
      BD *b = new BD[m_frameHeight * m_frameWidth];

      if (fread(r, sizeof(BD), m_frameHeight * m_frameWidth, m_inputFileHandle) != elementsToRead) {
        throw runtime_error("Cannot read the red component from the input file");
      }
      if (fread(g, sizeof(BD), m_frameHeight * m_frameWidth, m_inputFileHandle) != elementsToRead) {
        throw runtime_error("Cannot read the green component from the input file");
      }
      if (fread(b, sizeof(BD), m_frameHeight * m_frameWidth, m_inputFileHandle) != elementsToRead) {
        throw runtime_error("Cannot read the blue component from the input file");
      }

      // Compute the luma component
      for (int i = 0; i < m_frameHeight*m_frameWidth; i++) {
        const double Ey = (0.2126*r[i] + 0.7152*g[i] + 0.0722*b[i]) / static_cast<double>(m_maxPixelValue);
        const int Dy    = static_cast<int>((219*Ey + 16) * static_cast<double>(1 << (m_bitDepth - 8)) + 0.5);
        m_frameDataCurrent[i] = Clip3(0, m_maxPixelValue, Dy);
      }

      delete[] r;
      delete[] g;
      delete[] b;
    } else {
      // Read the current luma component
      if (fread(m_frameDataCurrent, sizeof(BD), m_frameHeight * m_frameWidth, m_inputFileHandle) != elementsToRead) {
        throw runtime_error("Cannot read from the input file");
      }

      // Move the file pointed over the chroma component
      if (fseek(m_inputFileHandle, m_bytesChroma, SEEK_CUR) != 0) {
        throw runtime_error("Cannot move the input file pointer beyond the chroma component");
      }
    }
  }

  void init(const int frameHeight, const int frameWidth, const int chromaFormat, const int startFrameIdx, const int bitDepth, const string &inputFileName, const bool isRgb = false)
  {
    m_frameHeight = frameHeight;
    m_frameWidth = frameWidth;
    m_chromaFormat = chromaFormat;
    m_maxPixelValue = (1 << bitDepth) - 1;
    m_bitDepth = bitDepth;
    m_isRgb = isRgb;

    switch (chromaFormat) {
    case 420:
      m_bytesChroma = (m_frameHeight * m_frameWidth) >> 1;
      break;
    case 422:
      m_bytesChroma = (m_frameHeight * m_frameWidth);
      break;
    case 444:
      m_bytesChroma = (m_frameHeight * m_frameWidth) << 1;
      break;
    default:
      throw logic_error("Undefined chroma subsampling format: " + to_string(chromaFormat));
    }
    m_bytesChroma *= sizeof(BD);

    m_bytesPerFrame = m_frameHeight * m_frameWidth * sizeof(BD) + m_bytesChroma;

    m_frameDataCurrent = new BD[m_frameWidth * m_frameHeight];

    if (!m_frameDataCurrent) {
      throw runtime_error("Cannot allocate memory for the pixel data current");
    }

    m_frameDataPrevious = new BD[m_frameWidth * m_frameHeight];

    if (!m_frameDataPrevious) {
      throw runtime_error("Cannot allocate memory for the pixel data previous");
    }

    m_inputFileHandle = fopen(inputFileName.c_str(), "rb");

    if (!m_inputFileHandle) {
      throw runtime_error("Cannot open the input file: " + inputFileName);
    }

    long long offset = static_cast<long long>(startFrameIdx) * static_cast<long long>(m_bytesPerFrame);
#if _WIN32 || _WIN64
    if (_fseeki64(m_inputFileHandle, offset, SEEK_SET) != 0) {
      throw runtime_error("Cannot move the file pointer to the start frame position: " + to_string(offset));
    }
#else
    if (fseek(m_inputFileHandle, offset, SEEK_SET) != 0) {
      throw runtime_error("Cannot move the file pointer to the start frame position: " + to_string(offset));
    }
#endif
  }

  void computeSpatialIndex(const int frameIdx)
  {
    double horizontalEdge, verticalEdge;
    double sumSquareGradientMag = 0.0, sumGradientMag = 0.0;
    int counter = 0;

    BD *lineUp = m_frameDataCurrent;
    BD *lineCu = m_frameDataCurrent + m_frameWidth;
    BD *lineDw = m_frameDataCurrent + (m_frameWidth << 1);

    for (int r = 1; r < m_frameHeight - 1; r++) {
      for (int c = 1; c < m_frameWidth - 1; c++, counter++) {
        // Horizontal edge
        horizontalEdge = static_cast<double>(( 1) * lineUp[c-1] + ( 2) * lineUp[c] + ( 1) * lineUp[c+1] +
                                             (-1) * lineDw[c-1] + (-2) * lineDw[c] + (-1) * lineDw[c+1]);
        horizontalEdge /= 8.0;

        // Vertical edge
        verticalEdge   = static_cast<double>((1) * lineUp[c-1] + (-1) * lineUp[c+1] +
                                             (2) * lineCu[c-1] + (-2) * lineCu[c+1] +
                                             (1) * lineDw[c-1] + (-1) * lineDw[c+1]);
        verticalEdge  /= 8.0;

        double currentGradientMag = sqrt(horizontalEdge*horizontalEdge + verticalEdge*verticalEdge);
        sumGradientMag          += currentGradientMag;
        sumSquareGradientMag    += currentGradientMag * currentGradientMag;
      }
      lineUp += m_frameWidth;
      lineCu += m_frameWidth;
      lineDw += m_frameWidth;
    }

    // Compute the standard deviation of all spatial indexes
    sumSquareGradientMag /= static_cast<double>(counter);
    sumGradientMag       /= static_cast<double>(counter);
    m_currentSpatialIdx   = sqrt(sumSquareGradientMag - sumGradientMag*sumGradientMag);
    m_maxSpatialIdx       = max<double>(m_maxSpatialIdx, m_currentSpatialIdx);
  }

  void computeTemporalIndex(const int frameIdx)
  {
    double sumSquareDifference = 0.0, sumDifference = 0.0;

    for (int r = 0; r < m_frameHeight; r++) {
      for (int c = 0; c < m_frameWidth; c++) {
        const double difference = static_cast<double>(m_frameDataCurrent[r*m_frameWidth + c] - m_frameDataPrevious[r*m_frameWidth + c]);
        sumSquareDifference   += difference*difference;
        sumDifference         += difference;
      }
    }

    sumSquareDifference /= static_cast<double>(m_frameHeight*m_frameWidth);
    sumDifference       /= static_cast<double>(m_frameHeight*m_frameWidth);
    m_currentTemporalIdx = sqrt(sumSquareDifference - sumDifference*sumDifference);
    m_maxTemporalIdx     = max(m_maxTemporalIdx, m_currentTemporalIdx);
  }
};

#endif