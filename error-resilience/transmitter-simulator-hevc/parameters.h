/*  transmitter-simulator-hevc, version 0.1
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
*/

#ifndef H_PARAMETERS_
#define H_PARAMETERS_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

/*!
 *
 *	\brief
 *	Models the parameters related to the transmission conditions
 *
 *	\author
 *	Matteo Naccari
 *
*/
class Parameters {
private:
  string m_bitstream_original, m_bitstream_transmitted, m_loss_pattern_file;
  int m_modality, m_offset;
  bool valid_line(const string& line);
  void check_parameters();

public:
  //! First constructor: the parameters are passed through command line
  Parameters(char** argv);

  //! Second constructor: the parameters are passed through a configuration file
  Parameters(char* argv);

  ~Parameters() {};

  const string& get_bitstream_original_filename() const { return m_bitstream_original; }
  const string& get_bitstream_transmitted_filename() const { return m_bitstream_transmitted; }
  const string& get_loss_pattern_filename() const { return m_loss_pattern_file; }
  int get_modality() const { return m_modality; }
  int get_offset() const { return m_offset; }
};

#endif

